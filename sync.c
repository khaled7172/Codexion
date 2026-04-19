/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/19 18:39:53 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	remove_from_queue(t_heap *h, int coder_id)
{
	int	i;

	i = 0;
	while (i < h->size)
	{
		if (h->data[i].coder_id == coder_id)
		{
			h->data[i] = h->data[h->size - 1];
			h->size--;
			return ;
		}
		i++;
	}
}

/*
** acquire_one: called with dongle->lock already held.
** Waits under dongle->lock until this coder is front of queue,
** dongle free, cooldown passed.
** On stop: cleans queue, unlocks dongle, returns 0.
** On success: pops queue, marks in_use, leaves lock held, returns 1.
*/
static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_sim			*sim;
	t_waiter		w;
	struct timespec	ts;
	int				stopped;

	sim = coder->sim;
	w.key = sched_key(coder, dongle);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&dongle->queue, w);
	while (1)
	{
		pthread_mutex_lock(&sim->stop_lock);
		stopped = sim->stop;
		pthread_mutex_unlock(&sim->stop_lock);
		if (stopped)
		{
			remove_from_queue(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
		if (dongle->queue.size > 0
			&& dongle->queue.data[0].coder_id == coder->id
			&& !dongle->in_use
			&& get_time_ms() >= dongle->ready_at)
			break ;
		ts.tv_sec = dongle->ready_at / 1000;
		ts.tv_nsec = (dongle->ready_at % 1000) * 1000000;
		pthread_cond_timedwait(&coder->cond, &dongle->lock, &ts);
	}
	heap_pop(&dongle->queue);
	dongle->in_use = 1;
	return (1);
}

static int	handle_single(t_coder *coder)
{
	t_dongle	*d;
	t_sim		*sim;
	int			stopped;

	d = coder->left;
	sim = coder->sim;
	pthread_mutex_lock(&d->lock);
	if (!acquire_one(coder, d))
		return (0);
	log_state(sim, coder->id, "has taken a dongle");
	while (1)
	{
		pthread_mutex_lock(&sim->stop_lock);
		stopped = sim->stop;
		pthread_mutex_unlock(&sim->stop_lock);
		if (stopped)
			break ;
		pthread_cond_wait(&coder->cond, &d->lock);
	}
	d->in_use = 0;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

static void	order_dongles(t_coder *c, t_dongle **first, t_dongle **second)
{
	if (c->left->id < c->right->id)
	{
		*first = c->left;
		*second = c->right;
	}
	else
	{
		*first = c->right;
		*second = c->left;
	}
}

/*
** Lock both dongles simultaneously without ABBA deadlock:
** Hold first->lock, trylock second. If trylock fails,
** drop first, yield, retry. ID ordering ensures no cycle.
*/
static int	lock_both(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_sim	*sim;
	int		stopped;

	sim = coder->sim;
	while (1)
	{
		pthread_mutex_lock(&sim->stop_lock);
		stopped = sim->stop;
		pthread_mutex_unlock(&sim->stop_lock);
		if (stopped)
			return (0);
		pthread_mutex_lock(&first->lock);
		if (pthread_mutex_trylock(&second->lock) == 0)
			return (1);
		pthread_mutex_unlock(&first->lock);
		ft_usleep(1, coder->sim);
	}
}

static void	rollback_first(t_dongle *first, t_sim *sim)
{
	pthread_mutex_lock(&first->lock);
	first->in_use = 0;
	first->ready_at = get_time_ms() + sim->cooldown;
	if (first->queue.size > 0)
		pthread_cond_signal(first->queue.data[0].cond);
	pthread_mutex_unlock(&first->lock);
}

int	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left == coder->right)
		return (handle_single(coder));
	order_dongles(coder, &first, &second);
	if (!lock_both(coder, first, second))
		return (0);
	if (!acquire_one(coder, first))
	{
		pthread_mutex_unlock(&second->lock);
		return (0);
	}
	if (!acquire_one(coder, second))
	{
		rollback_first(first, coder->sim);
		return (0);
	}
	pthread_mutex_unlock(&second->lock);
	pthread_mutex_unlock(&first->lock);
	log_state(coder->sim, coder->id, "has taken a dongle");
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (1);
}

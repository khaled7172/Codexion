/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/20 01:20:30 by kali             ###   ########.fr       */
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

static int	dongle_ready(t_coder *coder, t_dongle *d)
{
	return (d->queue.size > 0
		&& d->queue.data[0].coder_id == coder->id
		&& !d->in_use
		&& get_time_ms() >= d->ready_at);
}

static int	wait_for_dongle(t_coder *coder, t_dongle *dongle)
{
	t_sim			*sim;
	struct timespec	ts;
	int				stopped;

	sim = coder->sim;
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
		if (dongle_ready(coder, dongle))
			break ;
		ts.tv_sec = dongle->ready_at / 1000;
		ts.tv_nsec = (dongle->ready_at % 1000) * 1000000;
		pthread_cond_timedwait(&coder->cond, &dongle->lock, &ts);
	}
	return (1);
}

static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_waiter	w;

	w.key = sched_key(coder, dongle);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&dongle->queue, w);
	if (!wait_for_dongle(coder, dongle))
		return (0);
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

static void	release_on_stop(t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&second->lock);
	second->in_use = 0;
	second->ready_at = 0;
	pthread_mutex_unlock(&second->lock);
	pthread_mutex_lock(&first->lock);
	first->in_use = 0;
	first->ready_at = 0;
	if (first->queue.size > 0)
		pthread_cond_signal(first->queue.data[0].cond);
	pthread_mutex_unlock(&first->lock);
}

static int	acquire_second(t_coder *c, t_dongle *first, t_dongle *second)
{
	t_waiter	w;

	pthread_mutex_lock(&second->lock);
	w.key = sched_key(c, second);
	w.coder_id = c->id;
	w.cond = &c->cond;
	heap_push(&second->queue, w);
	if (!wait_for_dongle(c, second))
	{
		pthread_mutex_lock(&first->lock);
		first->in_use = 0;
		first->ready_at = 0;
		if (first->queue.size > 0)
			pthread_cond_signal(first->queue.data[0].cond);
		pthread_mutex_unlock(&first->lock);
		return (0);
	}
	heap_pop(&second->queue);
	second->in_use = 1;
	pthread_mutex_unlock(&second->lock);
	return (1);
}

int	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	t_waiter	w;

	if (coder->left == coder->right)
		return (handle_single(coder));
	order_dongles(coder, &first, &second);
	pthread_mutex_lock(&first->lock);
	w.key = sched_key(coder, first);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&first->queue, w);
	if (!wait_for_dongle(coder, first))
		return (0);
	heap_pop(&first->queue);
	first->in_use = 1;
	pthread_mutex_unlock(&first->lock);
	if (!acquire_second(coder, first, second))
		return (0);
	if (sim_stopped(coder->sim))
		return (release_on_stop(first, second), 0);
	log_state(coder->sim, coder->id, "has taken a dongle");
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (1);
}

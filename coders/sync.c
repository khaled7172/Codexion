/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/20 12:41:01 by khhammou         ###   ########.fr       */
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

static void	cond_sleep(t_coder *coder)
{
	pthread_mutex_lock(&coder->cond_lock);
	if (!coder->notified)
		pthread_cond_wait(&coder->cond, &coder->cond_lock);
	pthread_mutex_unlock(&coder->cond_lock);
}

static int	wait_for_dongle(t_coder *coder, t_dongle *dongle)
{
	while (1)
	{
		pthread_mutex_lock(&dongle->lock);
		if (sim_stopped(coder->sim))
		{
			remove_from_queue(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
		if (dongle_ready(coder, dongle))
			return (pthread_mutex_unlock(&dongle->lock), 1);
		coder->notified = 0;
		pthread_mutex_unlock(&dongle->lock);
		cond_sleep(coder);
	}
}

static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_waiter	w;

	w.key = sched_key(coder, dongle);
	w.coder_id = coder->id;
	w.notified = &coder->notified;
	pthread_mutex_lock(&dongle->lock);
	heap_push(&dongle->queue, w);
	pthread_mutex_unlock(&dongle->lock);
	if (!wait_for_dongle(coder, dongle))
		return (0);
	pthread_mutex_lock(&dongle->lock);
	heap_pop(&dongle->queue);
	dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->lock);
	return (1);
}

static int	handle_single(t_coder *coder)
{
	t_dongle	*d;

	d = coder->left;
	if (!acquire_one(coder, d))
		return (0);
	log_state(coder->sim, coder->id, "has taken a dongle");
	while (!sim_stopped(coder->sim))
	{
		pthread_mutex_lock(&coder->cond_lock);
		pthread_cond_wait(&coder->cond, &coder->cond_lock);
		pthread_mutex_unlock(&coder->cond_lock);
	}
	pthread_mutex_lock(&d->lock);
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

static void	signal_front(t_dongle *d, t_sim *sim)
{
	t_coder	*c;

	if (d->queue.size == 0)
		return ;
	c = &sim->coders[d->queue.data[0].coder_id - 1];
	pthread_mutex_lock(&c->cond_lock);
	c->notified = 1;
	pthread_cond_signal(&c->cond);
	pthread_mutex_unlock(&c->cond_lock);
}

static void	rollback(t_dongle *first, t_sim *sim)
{
	pthread_mutex_lock(&first->lock);
	first->in_use = 0;
	first->ready_at = 0;
	signal_front(first, sim);
	pthread_mutex_unlock(&first->lock);
}

int	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left == coder->right)
		return (handle_single(coder));
	order_dongles(coder, &first, &second);
	if (!acquire_one(coder, first))
		return (0);
	if (!acquire_one(coder, second))
		return (rollback(first, coder->sim), 0);
	if (sim_stopped(coder->sim))
	{
		rollback(second, coder->sim);
		rollback(first, coder->sim);
		return (0);
	}
	log_state(coder->sim, coder->id, "has taken a dongle");
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (1);
}

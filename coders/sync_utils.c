/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 20:07:00 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/22 20:11:08 by khhammou         ###   ########.fr       */
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

int	acquire_one(t_coder *coder, t_dongle *dongle)
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

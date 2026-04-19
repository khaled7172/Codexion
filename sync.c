/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/19 04:36:18 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_waiter		w;
	struct timespec	ts;

	w.key = sched_key(coder, dongle);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&dongle->queue, w);
	while (1)
	{
		if (coder->sim->stop)
			return (pthread_mutex_unlock(&dongle->lock), 0);
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

	d = coder->left;
	pthread_mutex_lock(&d->lock);
	if (!acquire_one(coder, d))
		return (0);
	log_state(coder->sim, coder->id, "has taken a dongle");
	while (!coder->sim->stop)
		pthread_cond_wait(&coder->cond, &d->lock);
	pthread_mutex_unlock(&d->lock);
	return (0);
}

static void	rollback_first(t_dongle *first, t_sim *sim)
{
	pthread_mutex_lock(&first->lock);
	first->in_use = 0;
	first->ready_at = get_time_ms() + sim->cooldown;
	if (first->queue.size > 0)
		pthread_cond_broadcast(first->queue.data[0].cond);
	pthread_mutex_unlock(&first->lock);
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

int	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left == coder->right)
		return (handle_single(coder));
	order_dongles(coder, &first, &second);
	pthread_mutex_lock(&first->lock);
	if (!acquire_one(coder, first))
		return (0);
	pthread_mutex_unlock(&first->lock);
	pthread_mutex_lock(&second->lock);
	if (!acquire_one(coder, second))
		return (rollback_first(first, coder->sim), 0);
	pthread_mutex_unlock(&second->lock);
	log_state(coder->sim, coder->id, "has taken a dongle");
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (1);
}

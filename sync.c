/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/18 02:13:16 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_sim		*sim;
	t_waiter	w;

	sim = coder->sim;
	w.key = sched_key(coder, dongle);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&dongle->queue, w);
	while (1)
	{
		if (sim->stop)
			return (pthread_mutex_unlock(&sim->state_lock), 0);
		if (dongle->queue.size > 0
			&& dongle->queue.data[0].coder_id == coder->id
			&& !dongle->in_use
			&& get_time_ms() >= dongle->ready_at)
			break ;
		pthread_cond_wait(&coder->cond, &sim->state_lock);
	}
	heap_pop(&dongle->queue);
	dongle->in_use = 1;
	return (1);
}

static int	handle_single(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	if (!acquire_one(coder, coder->left))
		return (pthread_mutex_unlock(&sim->state_lock), 0);
	log_state(sim, coder->id, "has taken a dongle");
	while (!sim->stop)
		pthread_cond_wait(&coder->cond, &sim->state_lock);
	pthread_mutex_unlock(&sim->state_lock);
	return (0);
}

static void	rollback_first(t_dongle *first, t_sim *sim)
{
	first->in_use = 0;
	first->ready_at = get_time_ms() + sim->cooldown;
	if (first->queue.size > 0)
		pthread_cond_signal(first->queue.data[0].cond);
	pthread_mutex_unlock(&sim->state_lock);
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
	t_sim		*sim;
	t_dongle	*first;
	t_dongle	*second;

	sim = coder->sim;
	if (coder->left == coder->right)
		return (handle_single(coder));
	order_dongles(coder, &first, &second);
	pthread_mutex_lock(&sim->state_lock);
	if (!acquire_one(coder, first))
		return (pthread_mutex_unlock(&sim->state_lock), 0);
	if (!acquire_one(coder, second))
		return (rollback_first(first, sim), 0);
	pthread_mutex_unlock(&sim->state_lock);
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (1);
}

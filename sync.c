/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/16 12:14:03 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}
static long	sched_key(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (sim->scheduler == EDF)
		return (coder->last_compile_time + sim->time_to_burnout);
	return (get_time_ms());
}

static int	acquire_one(t_coder *coder, t_dongle *dongle)
{
	t_sim		*sim;
	t_waiter	w;

	sim = coder->sim;
	w.key = sched_key(coder);
	w.coder_id = coder->id;
	w.cond = &coder->cond;
	heap_push(&dongle->queue, w);
	while (1)
	{
		if (sim_stopped(sim))
			return (0);
		if (dongle->queue.data[0].coder_id == coder->id
			&& !dongle->in_use
			&& get_time_ms() >= dongle->ready_at)
			break ;
		pthread_cond_wait(&coder->cond, &sim->state_lock);
	}
	heap_pop(&dongle->queue);
	dongle->in_use = 1;
	return (1);
}

static void	release_one(t_dongle *dongle, t_sim *sim)
{
	int	i;

	dongle->in_use = 0;
	dongle->ready_at = get_time_ms() + sim->cooldown;
	i = 0;
	while (i < dongle->queue.size)
	{
		pthread_cond_signal(dongle->queue.data[i].cond);
		i++;
	}
}
int	acquire_both_dongles(t_coder *coder)
{
	t_sim		*sim;
	t_dongle	*first;
	t_dongle	*second;

	sim = coder->sim;
	if (coder->left == coder->right)
	{
		pthread_mutex_lock(&sim->state_lock);
		if (!acquire_one(coder, coder->left))
			return (pthread_mutex_unlock(&sim->state_lock), 0);
		pthread_mutex_unlock(&sim->state_lock);
		log_state(sim, coder->id, "has taken a dongle");
		return (1);
	}
	if (coder->left->id < coder->right->id)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	pthread_mutex_lock(&sim->state_lock);
	if (!acquire_one(coder, first))
		return (pthread_mutex_unlock(&sim->state_lock), 0);
	if (!acquire_one(coder, second))
	{
		release_one(first, sim);
		return (pthread_mutex_unlock(&sim->state_lock), 0);
	}
	pthread_mutex_unlock(&sim->state_lock);
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (1);
}

void	release_both_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	release_one(coder->left, sim);
	if (coder->left != coder->right)
		release_one(coder->right, sim);
	pthread_mutex_unlock(&sim->state_lock);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/17                                     +#+#+#+#+#+   +#+ */
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


static long	sched_key(t_coder *coder, t_dongle *dongle)
{
	t_sim	*sim;
	long	key;

	sim = coder->sim;
	if (sim->scheduler == EDF)
	{
		key = coder->last_compile_time + sim->time_to_burnout;
		return (key * 1000 + coder->id);
	}
	key = sim->global_ticket++;
	pthread_mutex_unlock(&sim->state_lock);
	(void)dongle;
	return (key);
}

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
		if (sim_stopped(sim))
		{
			pthread_mutex_unlock(&sim->state_lock);
			return (0);
		}
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

/*
** FIX 3:
** - Only wake NEXT waiter (not everyone)
*/
static void	release_one(t_dongle *dongle, t_sim *sim)
{
	dongle->in_use = 0;
	dongle->ready_at = get_time_ms() + sim->cooldown;

	if (dongle->queue.size > 0)
		pthread_cond_signal(dongle->queue.data[0].cond);
}

int	acquire_both_dongles(t_coder *coder)
{
	t_sim		*sim;
	t_dongle	*first;
	t_dongle	*second;

	sim = coder->sim;

	/* Single coder edge case */
	if (coder->left == coder->right)
	{
		pthread_mutex_lock(&sim->state_lock);
		if (!acquire_one(coder, coder->left))
			return (pthread_mutex_unlock(&sim->state_lock), 0);
		log_state(sim, coder->id, "has taken a dongle");
		while (!sim->stop)
			pthread_cond_wait(&coder->cond, &sim->state_lock);
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}

	/* Prevent deadlock by ordering */
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
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
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

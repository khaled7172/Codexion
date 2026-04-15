/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sync.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 23:19:20 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 23:44:19 by kali             ###   ########.fr       */
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

/*
** Called while state_lock is already held.
** Both dongles must be free AND past their cooldown.
*/
int	can_compile(t_coder *coder)
{
	long	now;

	now = get_time_ms();
	if (coder->left->in_use || coder->right->in_use)
		return (0);
	if (now < coder->left->ready_at || now < coder->right->ready_at)
		return (0);
	return (1);
}

int	acquire_both_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	while (!can_compile(coder))
	{
		if (sim_stopped(sim))
		{
			pthread_mutex_unlock(&sim->state_lock);
			return (0);
		}
		pthread_cond_wait(&sim->state_cond, &sim->state_lock);
	}
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	pthread_mutex_unlock(&sim->state_lock);
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (1);
}

void	release_both_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	now = get_time_ms();
	pthread_mutex_lock(&sim->state_lock);
	coder->left->in_use = 0;
	coder->right->in_use = 0;
	coder->left->ready_at = now + sim->cooldown;
	coder->right->ready_at = now + sim->cooldown;
	pthread_cond_broadcast(&sim->state_cond);
	pthread_mutex_unlock(&sim->state_lock);
}

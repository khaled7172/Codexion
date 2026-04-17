/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 00:00:00 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/18 02:13:37 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	release_dongle(t_dongle *d, t_sim *sim)
{
	d->in_use = 0;
	d->ready_at = get_time_ms() + sim->cooldown;
	if (d->queue.size > 0)
		pthread_cond_signal(d->queue.data[0].cond);
}

void	release_both_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	release_dongle(coder->left, sim);
	if (coder->left != coder->right)
		release_dongle(coder->right, sim);
	pthread_mutex_unlock(&sim->state_lock);
}

static void	free_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		i++;
	}
	free(sim->coders);
}

static void	free_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		heap_free(&sim->dongles[i].queue);
		i++;
	}
	free(sim->dongles);
}

void	free_all(t_sim *sim)
{
	if (sim->coders)
		free_coders(sim);
	if (sim->dongles)
		free_dongles(sim);
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->state_lock);
	free(sim);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 02:34:26 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/20 02:34:27 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	release_dongle(t_dongle *d, long cooldown)
{
	pthread_mutex_lock(&d->lock);
	d->in_use = 0;
	d->ready_at = get_time_ms() + cooldown;
	if (d->queue.size > 0)
		pthread_cond_signal(d->queue.data[0].cond);
	pthread_mutex_unlock(&d->lock);
}

void	release_both_dongles(t_coder *coder)
{
	release_dongle(coder->left, coder->sim->cooldown);
	if (coder->left != coder->right)
		release_dongle(coder->right, coder->sim->cooldown);
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
		pthread_mutex_destroy(&sim->dongles[i].lock);
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
	pthread_cond_destroy(&sim->sleep_cond);
	pthread_mutex_destroy(&sim->sleep_mutex);
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->ticket_lock);
	free(sim);
}

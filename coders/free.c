/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 00:00:00 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/22 20:11:44 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	free_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		pthread_mutex_destroy(&sim->coders[i].cond_lock);
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

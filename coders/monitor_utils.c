/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 20:03:37 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/22 20:12:33 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wake_all(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_lock);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	pthread_mutex_lock(&sim->sleep_mutex);
	pthread_cond_broadcast(&sim->sleep_cond);
	pthread_mutex_unlock(&sim->sleep_mutex);
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->coders[i].cond_lock);
		sim->coders[i].notified = 1;
		pthread_cond_broadcast(&sim->coders[i].cond);
		pthread_mutex_unlock(&sim->coders[i].cond_lock);
		i++;
	}
}

void	set_burnout(t_sim *sim, int id, long now)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (!sim->stop)
	{
		sim->stop = 1;
		sim->burned_out = 1;
		pthread_mutex_lock(&sim->log_lock);
		printf("%ld %d burned out\n", now - sim->start_time, id);
		pthread_mutex_unlock(&sim->log_lock);
	}
	pthread_mutex_unlock(&sim->stop_lock);
}

int	all_done(t_sim *sim)
{
	int	i;

	if (sim->must_compile == 0)
		return (1);
	pthread_mutex_lock(&sim->stop_lock);
	i = 0;
	while (i < sim->num_coders)
	{
		if (sim->coders[i].compile_count < sim->must_compile)
			return (pthread_mutex_unlock(&sim->stop_lock), 0);
		i++;
	}
	pthread_mutex_unlock(&sim->stop_lock);
	return (1);
}

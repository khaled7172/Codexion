/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:40 by kali              #+#    #+#             */
/*   Updated: 2026/04/18 02:14:26 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	all_done(t_sim *sim)
{
	int	i;

	if (sim->must_compile == 0)
		return (0);
	i = 0;
	while (i < sim->num_coders)
	{
		if (sim->coders[i].compile_count < sim->must_compile)
			return (0);
		i++;
	}
	return (1);
}

static void	log_burnout(t_sim *sim, int id, long now)
{
	pthread_mutex_lock(&sim->log_lock);
	printf("%ld %d burned out\n", now - sim->start_time, id);
	pthread_mutex_unlock(&sim->log_lock);
}

static int	check_burnout(t_sim *sim)
{
	long	now;
	long	deadline;
	int		i;

	now = get_time_ms();
	i = 0;
	while (i < sim->num_coders)
	{
		deadline = sim->coders[i].last_compile_time + sim->time_to_burnout;
		if (now >= deadline)
		{
			pthread_mutex_lock(&sim->stop_lock);
			if (!sim->stop)
			{
				sim->stop = 1;
				log_burnout(sim, sim->coders[i].id, now);
			}
			pthread_mutex_unlock(&sim->stop_lock);
			return (1);
		}
		i++;
	}
	return (0);
}

static void	wake_all(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->state_lock);
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_cond_signal(&sim->coders[i].cond);
		i++;
	}
	pthread_mutex_unlock(&sim->state_lock);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		ft_usleep(1, sim);
		if (check_burnout(sim) || all_done(sim))
		{
			pthread_mutex_lock(&sim->stop_lock);
			if (!sim->stop)
				sim->stop = 1;
			pthread_mutex_unlock(&sim->stop_lock);
			wake_all(sim);
			return (NULL);
		}
	}
	return (NULL);
}

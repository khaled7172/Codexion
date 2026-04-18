/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:40 by kali              #+#    #+#             */
/*   Updated: 2026/04/18 22:55:14 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	all_done(t_sim *sim)
{
	int	i;
	int	count;

	if (sim->must_compile == 0)
		return (1);
	pthread_mutex_lock(&sim->state_lock);
	i = 0;
	while (i < sim->num_coders)
	{
		count = sim->coders[i].compile_count;
		if (count < sim->must_compile)
			return (pthread_mutex_unlock(&sim->state_lock), 0);
		i++;
	}
	pthread_mutex_unlock(&sim->state_lock);
	return (1);
}

static void	set_burnout(t_sim *sim, int id, long now)
{
	pthread_mutex_lock(&sim->stop_lock);
	if (!sim->stop)
	{
		sim->stop = 1;
		pthread_mutex_lock(&sim->log_lock);
		printf("%ld %d burned out\n", now - sim->start_time, id);
		pthread_mutex_unlock(&sim->log_lock);
	}
	pthread_mutex_unlock(&sim->stop_lock);
}

static int	check_burnout(t_sim *sim)
{
	long	now;
	long	last;
	int		i;

	now = get_time_ms();
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->state_lock);
		last = sim->coders[i].last_compile_time;
		pthread_mutex_unlock(&sim->state_lock);
		if (now >= last + sim->time_to_burnout)
		{
			set_burnout(sim, sim->coders[i].id, now);
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
		pthread_cond_broadcast(&sim->coders[i].cond);
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

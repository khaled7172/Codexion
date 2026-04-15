/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:40 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 23:44:30 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	all_done(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		if (sim->coders[i].compile_count < sim->must_compile)
			return (0);
		i++;
	}
	return (1);
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
			log_state(sim, sim->coders[i].id, "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

static void	set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_lock);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	pthread_cond_broadcast(&sim->state_cond);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		usleep(1000);
		if (check_burnout(sim) || all_done(sim))
		{
			set_stop(sim);
			return (NULL);
		}
	}
	return (NULL);
}

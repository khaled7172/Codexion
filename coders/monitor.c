/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:40 by kali              #+#    #+#             */
/*   Updated: 2026/04/22 20:13:24 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_sim *sim)
{
	long	now;
	long	last;
	int		i;

	now = get_time_ms();
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->stop_lock);
		last = sim->coders[i].last_compile_time;
		pthread_mutex_unlock(&sim->stop_lock);
		if (now >= last + sim->time_to_burnout)
		{
			set_burnout(sim, sim->coders[i].id, now);
			return (1);
		}
		i++;
	}
	return (0);
}

static void	check_cooldowns(t_sim *sim)
{
	t_coder	*c;
	int		i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		if (sim->dongles[i].queue.size > 0
			&& !sim->dongles[i].in_use
			&& get_time_ms() >= sim->dongles[i].ready_at)
		{
			c = &sim->coders[sim->dongles[i].queue.data[0].coder_id - 1];
			pthread_mutex_lock(&c->cond_lock);
			c->notified = 1;
			pthread_cond_signal(&c->cond);
			pthread_mutex_unlock(&c->cond_lock);
		}
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		ft_usleep(1, sim);
		check_cooldowns(sim);
		if (check_burnout(sim) || all_done(sim))
		{
			wake_all(sim);
			return (NULL);
		}
	}
	return (NULL);
}

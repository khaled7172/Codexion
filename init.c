/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:12:56 by kali              #+#    #+#             */
/*   Updated: 2026/04/19 18:37:51 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->num_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->num_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].ready_at = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
			return (0);
		if (!heap_init(&sim->dongles[i].queue, sim->num_coders))
			return (0);
		i++;
	}
	return (1);
}

static int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->num_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->num_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_time = sim->start_time;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % sim->num_coders];
		sim->coders[i].sim = sim;
		if (pthread_cond_init(&sim->coders[i].cond, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->ticket_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->state_lock, NULL) != 0)
		return (0);
	return (1);
}

int	init_sim(t_sim *sim, char **av)
{
	if (!parse_args(sim, av))
		return (0);
	sim->stop = 0;
	sim->start_time = get_time_ms();
	sim->global_ticket = 0;
	if (!init_mutexes(sim))
		return (0);
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	return (1);
}

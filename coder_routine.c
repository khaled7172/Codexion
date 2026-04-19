/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:10 by kali              #+#    #+#             */
/*   Updated: 2026/04/19 04:37:00 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	log_state(sim, coder->id, "is compiling");
	ft_usleep(sim->time_to_compile, sim);
	pthread_mutex_lock(&sim->stop_lock);
	coder->last_compile_time = get_time_ms();
	coder->compile_count++;
	pthread_mutex_unlock(&sim->stop_lock);
}

static void	do_debug(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	ft_usleep(coder->sim->time_to_debug, coder->sim);
}

static void	do_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is refactoring");
	ft_usleep(coder->sim->time_to_refactor, coder->sim);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		ft_usleep(coder->sim->time_to_compile / 2, coder->sim);
	while (!sim_stopped(coder->sim))
	{
		if (coder->id % 2 == 0)
			ft_usleep(1, coder->sim);
		if (!acquire_both_dongles(coder))
			return (NULL);
		do_compile(coder);
		release_both_dongles(coder);
		if (sim_stopped(coder->sim))
			return (NULL);
		do_debug(coder);
		if (sim_stopped(coder->sim))
			return (NULL);
		do_refactor(coder);
	}
	return (NULL);
}

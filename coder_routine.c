/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:10 by kali              #+#    #+#             */
/*   Updated: 2026/04/20 01:20:41 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->stop_lock);
	coder->last_compile_time = get_time_ms();
	pthread_mutex_unlock(&sim->stop_lock);
	log_state(sim, coder->id, "is compiling");
	ft_usleep(sim->time_to_compile, sim);
	pthread_mutex_lock(&sim->stop_lock);
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

static int	reached_quota(t_coder *coder)
{
	int	count;
	int	must;

	pthread_mutex_lock(&coder->sim->stop_lock);
	count = coder->compile_count;
	must = coder->sim->must_compile;
	pthread_mutex_unlock(&coder->sim->stop_lock);
	return (must > 0 && count >= must);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		ft_usleep(coder->sim->time_to_compile / 2, coder->sim);
	while (!sim_stopped(coder->sim))
	{
		if (reached_quota(coder))
			return (NULL);
		if (!acquire_both_dongles(coder))
			return (NULL);
		do_compile(coder);
		release_both_dongles(coder);
		do_debug(coder);
		do_refactor(coder);
	}
	return (NULL);
}

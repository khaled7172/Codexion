/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:10 by kali              #+#    #+#             */
/*   Updated: 2026/04/16 12:16:20 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	coder->last_compile_time = get_time_ms();
	log_state(sim, coder->id, "is compiling");
	usleep(sim->time_to_compile * 1000);
	coder->compile_count++;
}

static void	do_debug(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	usleep(coder->sim->time_to_debug * 1000);
}

static void	do_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is refactoring");
	usleep(coder->sim->time_to_refactor * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!sim_stopped(coder->sim))
	{
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

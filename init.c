/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:12:56 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:08:05 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_sim(t_sim *sim, char **av)
{
    sim->num_coders = atoi(av[1]);
    sim->time_to_burnout = atoi(av[2]);
    sim->time_to_compile = atoi(av[3]);
    sim->time_to_debug = atoi(av[4]);
    sim->time_to_refactor = atoi(av[5]);
    sim->must_compile = atoi(av[6]);
    sim->cooldown = atoi(av[7]);

	if (sim->num_coders <= 0
		|| sim->time_to_burnout <= 0
		|| sim->time_to_compile <= 0
		|| sim->time_to_debug <= 0
		|| sim->time_to_refactor <= 0
		|| sim->must_compile < 0)
		return (0);

	if (!parse_scheduler(av[8], &sim->scheduler))
		return (0);

	return (1);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:34 by kali              #+#    #+#             */
/*   Updated: 2026/04/16 12:08:12 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_scheduler(char *s, t_scheduler *out)
{
	if (strcmp(s, "fifo") == 0)
		*out = FIFO;
	else if (strcmp(s, "edf") == 0)
		*out = EDF;
	else
		return (0);
	return (1);
}

static int	validate_nums(char **av)
{
	int	i;

	i = 1;
	while (i <= 7)
	{
		if (!is_number(av[i]))
			return (0);
		i++;
	}
	return (1);
}

static int	validate_values(t_sim *sim)
{
	if (sim->num_coders <= 0)
		return (0);
	if (sim->time_to_burnout <= 0)
		return (0);
	if (sim->time_to_compile <= 0)
		return (0);
	if (sim->must_compile < 0)
		return (0);
	return (1);
}

int	parse_args(t_sim *sim, char **av)
{
	if (!validate_nums(av))
		return (0);
	sim->num_coders = atoi(av[1]);
	sim->time_to_burnout = (long)atoi(av[2]);
	sim->time_to_compile = (long)atoi(av[3]);
	sim->time_to_debug = (long)atoi(av[4]);
	sim->time_to_refactor = (long)atoi(av[5]);
	sim->must_compile = atoi(av[6]);
	sim->cooldown = (long)atoi(av[7]);
	if (!validate_values(sim))
		return (0);
	if (!parse_scheduler(av[8], &sim->scheduler))
		return (0);
	return (1);
}

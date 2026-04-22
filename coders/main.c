/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:15:07 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/22 23:15:08 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	start_threads(t_sim *sim)
{
	int	i;

	pthread_create(&sim->monitor, NULL, monitor_routine, sim);
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL,
			coder_routine, &sim->coders[i]);
		i++;
	}
}

void	join_threads(t_sim *sim)
{
	int	i;

	pthread_join(sim->monitor, NULL);
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_sim	*sim;

	if (argc != 9)
		return (fprintf(stderr, "Usage: ./codexion <n> <burnout> <compile>"
				" <debug> <refactor> <must_compile> <cooldown> <fifo|edf>\n"),
			1);
	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (1);
	memset(sim, 0, sizeof(t_sim));
	if (!init_sim(sim, argv))
		return (free(sim), 1);
	if (sim->must_compile == 0)
		return (free_all(sim), 0);
	start_threads(sim);
	join_threads(sim);
	free_all(sim);
	return (0);
}

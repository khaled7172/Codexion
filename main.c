/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:28 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:54:18 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	start_threads(t_sim *sim)
{
	int	i;
	
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
	
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

int	init_all(t_sim *sim, char **argv)
{
	if (!init_sim(sim, argv))
		return (0);
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	link_dongles(sim);
	return (1);
}


int	main(int argc, char **argv)
{
	t_sim	*sim;

	if (argc != 9)
		return (1);
	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (1);
	memset(sim, 0, sizeof(t_sim));
	if (!init_all(sim, argv))
		return (1);
	start_threads(sim);
	join_threads(sim);
	free_all(sim);
	return (0);
}
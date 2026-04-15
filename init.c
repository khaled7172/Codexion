/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:12:56 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:54:28 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_coders(t_sim *sim)
{
    int i;
    
	sim->coders = malloc(sizeof(t_coder) * sim->num_coders);
	if (!sim->coders)
    return (0);
    
	i = 0;
	while (i < sim->num_coders)
	{
        sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_time = 0;
		sim->coders[i].sim = sim;
        
		i++;
	}
	return (1);
}
int	init_dongles(t_sim *sim)
{
    int i;
    
	sim->dongles = malloc(sizeof(t_dongle) * sim->num_coders);
	if (!sim->dongles)
    return (0);
    
	i = 0;
	while (i < sim->num_coders)
	{
        sim->dongles[i].id = i;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
        return (0);
		i++;
	}
	return (1);
}
void	link_dongles(t_sim *sim)
{
    int i;
    
	i = 0;
	while (i < sim->num_coders)
	{
        sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % sim->num_coders];
		i++;
	}
}
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
    if (!init_dongles(sim))
        return (0);
    if (!init_coders(sim))
        return (0);
    link_dongles(sim);

    return (1);
}
void	free_all(t_sim *sim)
{
	int i;
    
	if (sim->dongles)
	{
        i = 0;
		while (i < sim->num_coders)
		{
            pthread_mutex_destroy(&sim->dongles[i].lock);
			i++;
		}
		free(sim->dongles);
	}
    
	if (sim->coders)
    free(sim->coders);
}

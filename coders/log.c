/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:24 by kali              #+#    #+#             */
/*   Updated: 2026/04/21 23:03:27 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int id, char *msg)
{
	long	ts;

	if (sim->burned_out && strcmp(msg, "burned out") != 0)
		return ;
	pthread_mutex_lock(&sim->log_lock);
	if (sim->burned_out && strcmp(msg, "burned out") != 0)
		return (pthread_mutex_unlock(&sim->log_lock), (void)0);
	ts = get_time_ms() - sim->start_time;
	printf("%ld %d %s\n", ts, id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

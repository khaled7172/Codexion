/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 02:34:45 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/20 02:34:46 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int id, char *msg)
{
	long	ts;
	int		burned;

	pthread_mutex_lock(&sim->log_lock);
	pthread_mutex_lock(&sim->stop_lock);
	burned = sim->burned_out;
	pthread_mutex_unlock(&sim->stop_lock);
	if (burned && strcmp(msg, "burned out") != 0)
		return (pthread_mutex_unlock(&sim->log_lock), (void)0);
	ts = get_time_ms() - sim->start_time;
	printf("%ld %d %s\n", ts, id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

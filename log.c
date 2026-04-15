/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:24 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 23:44:06 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int id, char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->log_lock);
	ts = get_time_ms() - sim->start_time;
	printf("%ld %d %s\n", ts, id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

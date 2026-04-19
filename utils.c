/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 01:19:17 by kali              #+#    #+#             */
/*   Updated: 2026/04/20 01:19:20 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)(tv.tv_sec * 1000) + (long)(tv.tv_usec / 1000));
}

int	is_number(char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	sim_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_lock);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (val);
}

long	sched_key(t_coder *coder, t_dongle *dongle)
{
	t_sim	*sim;
	long	key;

	sim = coder->sim;
	(void)dongle;
	if (sim->scheduler == EDF)
	{
		key = coder->last_compile_time + sim->time_to_burnout;
		return (key * 1000 + coder->id);
	}
	pthread_mutex_lock(&sim->ticket_lock);
	key = sim->global_ticket++;
	pthread_mutex_unlock(&sim->ticket_lock);
	return (key);
}

void	ft_usleep(long ms, t_sim *sim)
{
	struct timeval	tv;
	struct timespec	ts;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec + ms / 1000;
	ts.tv_nsec = tv.tv_usec * 1000 + (ms % 1000) * 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}
	pthread_mutex_lock(&sim->sleep_mutex);
	if (!sim->stop)
		pthread_cond_timedwait(&sim->sleep_cond, &sim->sleep_mutex, &ts);
	pthread_mutex_unlock(&sim->sleep_mutex);
}

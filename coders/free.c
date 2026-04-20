/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 00:00:00 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/20 12:30:44 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	notify_front(t_dongle *d, t_sim *sim)
{
	t_coder	*c;

	if (d->queue.size == 0)
		return ;
	c = &sim->coders[d->queue.data[0].coder_id - 1];
	pthread_mutex_lock(&c->cond_lock);
	c->notified = 1;
	pthread_cond_signal(&c->cond);
	pthread_mutex_unlock(&c->cond_lock);
}

static void	release_dongle(t_dongle *d, long cooldown, t_sim *sim)
{
	pthread_mutex_lock(&d->lock);
	d->in_use = 0;
	d->ready_at = get_time_ms() + cooldown;
	notify_front(d, sim);
	pthread_mutex_unlock(&d->lock);
}

void	release_both_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	release_dongle(coder->left, sim->cooldown, sim);
	if (coder->left != coder->right)
		release_dongle(coder->right, sim->cooldown, sim);
}

static void	free_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		pthread_mutex_destroy(&sim->coders[i].cond_lock);
		i++;
	}
	free(sim->coders);
}

static void	free_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		heap_free(&sim->dongles[i].queue);
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	free(sim->dongles);
}

void	free_all(t_sim *sim)
{
	if (sim->coders)
		free_coders(sim);
	if (sim->dongles)
		free_dongles(sim);
	pthread_cond_destroy(&sim->sleep_cond);
	pthread_mutex_destroy(&sim->sleep_mutex);
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->ticket_lock);
	free(sim);
}

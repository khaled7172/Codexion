/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   release.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 20:02:31 by khhammou          #+#    #+#             */
/*   Updated: 2026/04/22 20:14:49 by khhammou         ###   ########.fr       */
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

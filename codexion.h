/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:17 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:56:03 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	int				id;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	long			last_compile_time;
	int				compile_count;

	t_dongle		*left;
	t_dongle		*right;

	pthread_t		thread;
	struct s_sim	*sim;
}	t_coder;


typedef struct s_sim
{
	int				num_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				must_compile;
	long			cooldown;
	t_scheduler		scheduler;

	int				stop;
	pthread_mutex_t	stop_lock;

	pthread_mutex_t	log_lock;

	t_coder			*coders;
	t_dongle		*dongles;
}	t_sim;


int	init_sim(t_sim *sim, char **argv);
long	ft_atol(char *s);
int	parse_scheduler(char *s, t_scheduler *out);
int	is_number(char *s);
void *coder_routine(void *arg);
void	free_all(t_sim *sim);
int	init_dongles(t_sim *sim);
void	link_dongles(t_sim *sim);
int	init_coders(t_sim *sim);

#endif
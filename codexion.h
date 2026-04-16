/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:17 by kali              #+#    #+#             */
/*   Updated: 2026/04/16 17:10:37 by kali             ###   ########.fr       */
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


typedef struct s_waiter
{
	long			key;
	int				coder_id;
	pthread_cond_t	*cond;
}	t_waiter;


typedef struct s_heap
{
	t_waiter		*data;
	int				size;
	int				cap;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			ready_at;
	long			ticket;
	t_heap			queue;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	long			last_compile_time;
	int				compile_count;
	pthread_cond_t	cond;
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
	long			start_time;
	int				stop;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
	pthread_t		monitor;
	t_coder			*coders;
	t_dongle		*dongles;
}	t_sim;

/* utils.c */
long		get_time_ms(void);
int			is_number(char *s);

/* parse.c */
int			parse_args(t_sim *sim, char **av);

/* init.c */
int			init_sim(t_sim *sim, char **av);
void		free_all(t_sim *sim);

/* log.c */
void		log_state(t_sim *sim, int id, char *msg);

/* scheduler.c */
int			heap_init(t_heap *h, int cap);
void		heap_free(t_heap *h);
void		heap_push(t_heap *h, t_waiter w);
t_waiter	heap_pop(t_heap *h);

void		ft_usleep(long ms, t_sim *sim);

/* sync.c */
int			acquire_both_dongles(t_coder *coder);
void		release_both_dongles(t_coder *coder);
int			sim_stopped(t_sim *sim);

/* monitor.c */
void		*monitor_routine(void *arg);

/* coder_routine.c */
void		*coder_routine(void *arg);

/* main.c */
void		start_threads(t_sim *sim);
void		join_threads(t_sim *sim);

#endif

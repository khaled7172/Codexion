# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/04/20 01:27:34 by khhammou          #+#    #+#              #
#    Updated: 2026/04/22 20:09:54 by khhammou         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -std=c89

SRCS = coders/free.c \
		coders/init.c \
		coders/log.c \
		coders/main.c \
		coders/monitor.c \
		coders/parse.c \
		coders/scheduler.c \
		coders/sync.c \
		coders/utils.c \
		coders/coder_routine.c \
		coders/monitor_utils.c \
		coders/release.c \
		coders/sync_utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

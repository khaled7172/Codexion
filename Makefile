NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -std=c89

SRCS = main.c \
	   init.c \
	   free.c \
	   parse.c \
	   utils.c \
	   log.c \
	   scheduler.c \
	   sync.c \
	   monitor.c \
	   coder_routine.c

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

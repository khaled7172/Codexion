/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khhammou <khhammou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:37 by kali              #+#    #+#             */
/*   Updated: 2026/04/20 12:32:28 by khhammou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *h, int cap)
{
	h->data = malloc(sizeof(t_waiter) * cap);
	if (!h->data)
		return (0);
	h->size = 0;
	h->cap = cap;
	return (1);
}

void	heap_free(t_heap *h)
{
	free(h->data);
	h->data = NULL;
	h->size = 0;
}

static void	sift_down(t_heap *h, int i)
{
	int			left;
	int			smallest;
	t_waiter	tmp;

	while (1)
	{
		left = 2 * i + 1;
		smallest = i;
		if (left < h->size && h->data[left].key < h->data[smallest].key)
			smallest = left;
		if (left + 1 < h->size
			&& h->data[left + 1].key < h->data[smallest].key)
			smallest = left + 1;
		if (smallest == i)
			break ;
		tmp = h->data[i];
		h->data[i] = h->data[smallest];
		h->data[smallest] = tmp;
		i = smallest;
	}
}

void	heap_push(t_heap *h, t_waiter w)
{
	int			i;
	int			parent;
	t_waiter	tmp;

	if (h->size >= h->cap)
		return ;
	h->data[h->size] = w;
	h->size++;
	i = h->size - 1;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (h->data[parent].key <= h->data[i].key)
			break ;
		tmp = h->data[parent];
		h->data[parent] = h->data[i];
		h->data[i] = tmp;
		i = parent;
	}
}

t_waiter	heap_pop(t_heap *h)
{
	t_waiter	top;

	top = h->data[0];
	h->size--;
	if (h->size > 0)
	{
		h->data[0] = h->data[h->size];
		sift_down(h, 0);
	}
	return (top);
}

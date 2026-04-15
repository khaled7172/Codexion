/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:34 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:06:54 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// int	is_number(char *s)
// {
// 	int i;

// 	i = 0;
// 	if (!s || !s[0])
// 		return (0);
// 	while (s[i])
// 	{
// 		if (s[i] < '0' || s[i] > '9')
// 			return (0);
// 		i++;
// 	}
// 	return (1);
// }
// long	ft_atol(char *s)
// {
// 	long res;
// 	int i;

// 	res = 0;
// 	i = 0;
// 	while (s[i])
// 	{
// 		res = res * 10 + (s[i] - '0');
// 		i++;
// 	}
// 	return (res);
// }
int	parse_scheduler(char *s, t_scheduler *out)
{
	if (strcmp(s, "fifo") == 0)
		*out = FIFO;
	else if (strcmp(s, "edf") == 0)
		*out = EDF;
	else
		return (0);
	return (1);
}

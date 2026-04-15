/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:13:34 by kali              #+#    #+#             */
/*   Updated: 2026/04/15 19:09:25 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

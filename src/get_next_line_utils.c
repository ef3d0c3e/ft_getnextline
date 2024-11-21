/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.h                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgamba <marvin@student.42perpignan.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 11:54:01 by lgamba            #+#    #+#             */
/*   Updated: 2024/11/05 17:50:12 by lgamba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "get_next_line.h"
#include <stdlib.h>

struct s_gnl_data	*__gnl(void)
{
	static struct s_gnl_data	data = {0, 0, 0};

	return (&data);
}

/* Copies memory from src to dest (n bytes)
 * NOTE: dest and src should ne `restrict` */
void	*__gnl_memcpy(void *dest, const void *src, size_t n)
{
	size_t						i;

	i = 0;
	while (i != n)
	{
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
		++i;
	}
	return (dest);
}

void	*__gnl_realloc(void *p, size_t origsz, size_t newsz)
{
	void	*q;

	if (newsz <= origsz)
		return (p);
	q = malloc(newsz);
	if (!q)
	{
		free(p);
		return (q);
	}
	__gnl_memcpy(q, p, origsz);
	if (p)
		free(p);
	return (q);
}

void	*__gnl_memnchr(const void *mem, int c, size_t len)
{
	const unsigned char	*m;

	m = mem;
	while (len != 0)
	{
		if (*m == (unsigned char)c)
			return ((void *)m);
		++m;
		--len;
	}
	return (0);
}

int	__gnl_at_least(struct s_gnl *gnl, size_t at_least)
{
	size_t	newsz;

	if (gnl->line_cap >= at_least)
		return (1);
	newsz = (gnl->line_cap + !gnl->line_cap) << 1;
	while (newsz < at_least)
		newsz <<= 1;
	gnl->line = __gnl_realloc(gnl->line, gnl->line_cap, newsz);
	gnl->line_cap = newsz;
	return (!!gnl->line);
}

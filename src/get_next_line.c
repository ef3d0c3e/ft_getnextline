/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgamba <marvin@student.42perpignan.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 11:54:01 by lgamba            #+#    #+#             */
/*   Updated: 2024/11/05 17:50:12 by lgamba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "get_next_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

struct s_gnl_data	*__gnl(void);
void				*__gnl_memcpy(void *dest, const void *src, size_t n);
void				*__gnl_realloc(void *p, size_t origsz, size_t newsz);
void				*__gnl_memnchr(const void *mem, int c, size_t len);
/**
 * @brief Ensure the internal line buffer has enough capacity to store
 * @ref at_least.
 *
 * @param gnl The gnl structure
 * @param at_least Ensures the internal buffer can hold @ref at_least bytes
 *
 * @returns 0 on __gnl_realloc failure, in such case the gnl structure should be
 * freed.
 */
int					__gnl_at_least(struct s_gnl *gnl, size_t at_least);

/* Cleans the gnl structure inside the global @ref __gnl_data.
 * If @p gnl is NULL, the entire structure is cleared */
static void	cleanup(struct s_gnl *gnl)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (i++ < __gnl()->size)
	{
		if (!gnl)
		{
			free(__gnl()->data[i - 1].line);
			free(&__gnl()->data[i - 1]);
			continue ;
		}
		else if (&__gnl()->data[i - 1] != gnl)
			continue ;
		j = i - 1;
		while (++j < __gnl()->size)
			__gnl()->data[j - 1] = __gnl()->data[j];
		--__gnl()->size;
		break ;
	}
	if (gnl)
		free(gnl->line);
	if (!__gnl()->size || !gnl)
		return (__gnl()->size = 0, __gnl()->capacity = 0,
			free(__gnl()->data));
}

/* Get the gnl data for a file descriptor, either by retrieving already existing
 * data, or by creating a new one */
static struct s_gnl	*get_data(int fd)
{
	size_t			i;

	i = 0;
	while (i++ < __gnl()->size)
		if (__gnl()->data[i - 1].fd == fd)
			return (&__gnl()->data[i - 1]);
	if (__gnl()->size >= __gnl()->capacity)
	{
		__gnl()->data = __gnl_realloc(__gnl()->data, __gnl()->capacity
			* sizeof(struct s_gnl),
			((__gnl()->capacity + !__gnl()->capacity) << 1)
			* sizeof(struct s_gnl));
		if (!__gnl()->data)
			return (NULL);
		__gnl()->capacity = (__gnl()->capacity + !__gnl()->capacity) << 1;
	}
	i = 0;
	while (i < sizeof(struct s_gnl))
		((unsigned char *)&__gnl()->data[__gnl()->size])[i++] = 0;
	return (&__gnl()->data[__gnl()->size++]);
}

/**
 * Copies the read buffer to the internal line buffer
 *
 * Return value:
 *  - 0: Malloc failed, data has been cleaned up
 *  - 1: A line has been read, return it to caller
 *  - 2: Partially added to the line buffer, continue reading
 */
static int	copy_buffer(struct s_gnl *gnl)
{
	const void		*end;

	if (!__gnl_at_least(gnl, gnl->line_sz + gnl->nb_read + 1))
		return (cleanup(gnl), 0);
	end = __gnl_memnchr(gnl->buffer, '\n', gnl->nb_read);
	if (end)
	{
		return (__gnl_memcpy(gnl->line + gnl->line_sz, gnl->buffer,
				(char *)end - gnl->buffer + 1),
			gnl->line[gnl->line_sz + (char *)end - gnl->buffer + 1] = 0,
			gnl->buf_pos = (char *)end - gnl->buffer + 1,
			gnl->line_sz = 0, 1);
	}
	else if (!gnl->nb_read)
	{
		if (gnl->line_sz)
			gnl->line[gnl->line_sz] = 0;
		else
			return (gnl->need_clean = 1, free(gnl->line), gnl->line = 0, 1);
		return (gnl->need_clean = 1, 1);
	}
	else
		return (__gnl_memcpy(gnl->line + gnl->line_sz, gnl->buffer,
				gnl->nb_read), gnl->line_sz += gnl->nb_read, 2);
}

/**
 * Processes the leftover data in the read buffer
 *
 * Return value:
 *  - 0: Malloc failed, data has been cleaned up
 *  - 1: A line has been read from leftover, return it to caller
 *  - 2: Partial/Nothing added to the line, continue reading
 */
static int	process_carry(struct s_gnl *gnl)
{
	char	*end;

	if (gnl->buf_pos != 0)
	{
		if (!__gnl_at_least(gnl, BUFFER_SIZE + 1))
			return (cleanup(gnl), 0);
		end = (char *)__gnl_memnchr(gnl->buffer + gnl->buf_pos,
				'\n', gnl->nb_read - gnl->buf_pos);
		if (end)
			return (gnl->line_sz = end - (gnl->buffer + gnl->buf_pos),
				__gnl_memcpy(gnl->line, gnl->buffer + gnl->buf_pos,
					gnl->line_sz + 1),
				gnl->buf_pos += gnl->line_sz + 1,
				gnl->line[gnl->line_sz + 1] = 0, 1);
		gnl->line_sz += gnl->nb_read - gnl->buf_pos;
		__gnl_memcpy(gnl->line, gnl->buffer + gnl->buf_pos,
			gnl->nb_read - gnl->buf_pos);
		gnl->buf_pos = 0;
	}
	return (2);
}

char	*get_next_line(int fd)
{
	struct s_gnl	*gnl;
	int				ret;

	gnl = get_data(fd);
	if (!gnl)
		return (cleanup(gnl), NULL);
	gnl->line_sz = 0;
	ret = process_carry(gnl);
	if (ret != 2)
		return ((char *)(ret * (unsigned long int)gnl->line));
	while (!gnl->need_clean)
	{
		gnl->nb_read = read(fd, gnl->buffer, BUFFER_SIZE);
		if (gnl->nb_read < 0)
			return (cleanup(gnl), NULL);
		ret = copy_buffer(gnl);
		if (ret != 2)
			return ((char *)(ret * (unsigned long int)gnl->line));
	}
	return (cleanup(gnl), NULL);
}

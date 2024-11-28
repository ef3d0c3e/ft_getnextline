/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgamba <marvin@student.42perpignan.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 11:54:01 by lgamba            #+#    #+#             */
/*   Updated: 2024/11/05 17:50:12 by lgamba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#define _GNL_INTERNAL
#include "get_next_line_bonus.h"
#include <stdlib.h>
#include <unistd.h>

/*
#include <link.h>
#include <malloc.h>
#include <dlfcn.h>
#include <sys/cdefs.h>

void free(void *p)
{
	printf("Free called for p=%p@%zu\n", p, malloc_usable_size(p));
	void *handle = dlopen("libc.so.6", RTLD_NOW);
	void (*ofree)(void*) = dlsym(handle, "free");
	if (p)
	{
		size_t s = malloc_usable_size(p);
		for (size_t i = 0; i < s; ++i)
			((unsigned char*)p)[i] = 0;
	}
	ofree(p);
}
*/

/* Cleans the gnl structure inside the global @ref __gnl_data.
 * If @p gnl is NULL, the entire structure is cleared */
static void	cleanup(struct s_gnl *gnl, int line)
{
	size_t	i;

	i = 0;
	while (i++ < __gnl()->size)
	{
		if ((!gnl && (free(__gnl()->data[i - 1].line),
					free(&__gnl()->data[i - 1]), 1))
			|| &__gnl()->data[i - 1] != gnl)
			continue ;
		((line && gnl && gnl->line) && (free(gnl->line), gnl->line = 0, 0));
		while (i++ < __gnl()->size)
			__gnl()->data[i - 2] = __gnl()->data[i - 1];
		--__gnl()->size;
	}
	(void)((!__gnl()->size || !gnl) && ((void)(__gnl()->data && (free(__gnl()
		->data), 1)), 1) && (__gnl()->size = 0, __gnl()->capacity = 0,
		__gnl()->data = 0, 1));
}

/* Get the gnl data for a file descriptor, either by retrieving already existing
 * data, or by creating a new one */
static struct s_gnl	*get_data(int fd)
{
	size_t			i;

	if (fd < 0)
		return (NULL);
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
	__gnl()->data[__gnl()->size].fd = fd;
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
		return (cleanup(gnl, 1), 0);
	end = __gnl_memnchr(gnl->buffer, '\n', gnl->nb_read);
	if (end)
		return (__gnl_memcpy(gnl->line + gnl->line_sz, gnl->buffer,
				(char *)end - gnl->buffer + 1),
			gnl->line[gnl->line_sz + (char *)end - gnl->buffer + 1] = 0,
			gnl->buf_pos = (char *)end - gnl->buffer + 1,
			gnl->line_sz = 0, 1);
	else if (!gnl->nb_read)
		return ((gnl->line_sz && (gnl->need_clean = 1,
					gnl->line[gnl->line_sz] = 0, 1)) || (cleanup(gnl, 1), 0));
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

	if (gnl->buf_pos == 0)
		return (2);
	if (!__gnl_at_least(gnl, BUFFER_SIZE + 1))
		return (cleanup(gnl, 1), 0);
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
	return (2);
}

char	*get_next_line(int fd)
{
	struct s_gnl	*gnl;
	int				ret;

	gnl = get_data(fd);
	if (!gnl)
		return (cleanup(gnl, 0), NULL);
	gnl->line_sz = 0;
	ret = process_carry(gnl);
	if (ret == 0)
		return (0);
	else if (ret == 1)
		return (gnl->line_cap = 0, gnl->line_sz = 0, gnl->line);
	while (!gnl->need_clean)
	{
		gnl->nb_read = read(fd, gnl->buffer, BUFFER_SIZE);
		if (gnl->nb_read < 0)
			return (cleanup(gnl, 1), NULL);
		ret = copy_buffer(gnl);
		if (ret == 0)
			return (0);
		else if (ret == 1)
			return (gnl->line_cap = 0, gnl->line_sz = 0, gnl->line);
	}
	return (cleanup(gnl, 0), NULL);
}

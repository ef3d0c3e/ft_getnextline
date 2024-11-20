#include "get_next_line.h"
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

struct s_gnl	__gnl_init(int fd);
void			*__gnl_memcpy(void *dest, const void *src, size_t n);
void			*__gnl_realloc(void *p, size_t origsz, size_t newsz);
void			*__gnl_memnchr(const void *mem, int c, size_t len);
int				__gnl_at_least(struct s_gnl *gnl, size_t at_least);

/**
 * @brief Global variable holding all the file descriptor data
 */
static struct s_gnl_data __gnl_data = { 0, 0, 0 };

/* Cleans the gnl structure inside the global @ref __gnl_data
 * If @p gnl is NULL, the entire structure is cleared */
static void	cleanup(struct s_gnl* gnl)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (i < __gnl_data.size)
	{
		if (!gnl)
		{
			free(__gnl_data.data[i].line);
			free(&__gnl_data.data[i]);
			continue;
		}
		else if (&__gnl_data.data[i] != gnl)
			continue;

		j = i;
		while (++j < __gnl_data.size)
			__gnl_data.data[j - 1] = __gnl_data.data[j];
		--__gnl_data.size;
		break;
	}
	if (!gnl)
		return (free(__gnl_data.data));
	free(gnl->line);
	if (!__gnl_data.size)
	{
		__gnl_data.capacity = 0;
		free(__gnl_data.data);
	}
}

/* Get the data, either by retreiving already existing data, or by creating a
 * new one */
static struct s_gnl *get_data(int fd)
{
	size_t			i;
	struct s_gnl	*data;

	i = 0;
	while (i < __gnl_data.size)
		if (__gnl_data.data[i].fd == fd)
			return (&__gnl_data.data[i++]);
	if (__gnl_data.size >= __gnl_data.capacity)
	{
		__gnl_data.data = __gnl_realloc(__gnl_data.data, __gnl_data.capacity
				* sizeof(struct s_gnl),
				((__gnl_data.capacity + !__gnl_data.capacity) << 1)
				* sizeof(struct s_gnl));
		if (!__gnl_data.data)
			return (NULL);
		__gnl_data.capacity <<= 1;
	}
	__gnl_data.data[__gnl_data.size] = __gnl_init(fd);
	++__gnl_data.size;
	return (&__gnl_data.data[__gnl_data.size - 1]);
}

static int copy_buffer(struct s_gnl *gnl, size_t nb_read)
{
	const void		*end;

	if (!__gnl_at_least(gnl, gnl->line_sz + nb_read + 1))
		return (cleanup(gnl), 0);
	end = __gnl_memnchr(gnl->buffer, '\n', nb_read);
	if (end)
		return (__gnl_memcpy(gnl->line + gnl->line_sz, gnl->buffer, (char *)end - gnl->buffer + 1),
				gnl->line[gnl->line_sz + (char *)end - gnl->buffer + 1] = 0,
				gnl->buf_pos = (char *)end - gnl->buffer + 1,
				gnl->line_sz = 0, 1);
	else if (!nb_read)
		return (gnl->line[gnl->line_sz] = 0, gnl->need_clean = 1, 1);
	else
		return (__gnl_memcpy(gnl->line + gnl->line_sz, gnl->buffer, nb_read),
				gnl->line_sz += nb_read, 2);
}

static int process_carry(struct s_gnl *gnl)
{
	char	*end;

	if (gnl->buf_pos != 0)
	{
		if (!__gnl_at_least(gnl, BUFFER_SIZE + 1))
			return (0);
		end = (char *)__gnl_memnchr(gnl->buffer + gnl->buf_pos,
				'\n', BUFFER_SIZE - gnl->buf_pos);
		if (end)
			return (gnl->line_sz = end - (gnl->buffer + gnl->buf_pos),
					__gnl_memcpy(gnl->line, gnl->buffer + gnl->buf_pos,
						gnl->line_sz), gnl->buf_pos += gnl->line_sz,
					gnl->line[gnl->line_sz] = 0, 1);
		__gnl_memcpy(gnl->line, gnl->buffer + gnl->buf_pos,
				BUFFER_SIZE - gnl->buf_pos);
		gnl->buf_pos = 0;
	}
	return (2);
}

char	*get_next_line(int fd)
{
	struct s_gnl	*data;
	ssize_t			nb_read;
	size_t			line_len;
	int				ret;

	data = get_data(fd);
	if (!data || data->need_clean)
		return (cleanup(data), NULL);
	data->line_sz = 0;
	ret = process_carry(data);
	if (ret != 2)
		return ((char *)(ret * (unsigned long int)data->line));
	while (1)
	{
		nb_read = read(fd, data->buffer, BUFFER_SIZE);
		if (nb_read < 0)
			return (cleanup(data), NULL);
		ret = copy_buffer(data, nb_read);
		if (ret != 2)
			return ((char *)(ret * (unsigned long int)data->line));
	}
	return (NULL);
}

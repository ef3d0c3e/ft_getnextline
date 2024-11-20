#include "get_next_line.h"
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

void	*__gnl_memcpy(void *dest, const void *src, size_t n);
void	*__gnl_realloc(void *p, size_t origsz, size_t newsz);
void	*__gnl_memnchr(const void *mem, int c, size_t len);

static struct s_gnl* insert_or_remove(
				struct s_gnl_data* data,
				int fd,
				size_t at,
				int remove
				)
{
	size_t	i;

	i = 0;
	if (remove)
	{
		free(data->data[at].line);
		while (at + ++i < data->size)
			data->data[at + i - 1] = data->data[at + i];
		--data->size;
		return (0);
	}
	if (data->size <= data->capacity)
	{
		__gnl_realloc(data->data, sizeof(struct s_gnl) * data->capacity,
				sizeof(struct s_gnl) * data->capacity * 2);
		data->capacity *= 2;
	}
	data->data[data->size++] = (struct s_gnl){fd, 0, 0};
	return (&data->data[data->size - 1]);
}

static struct s_gnl *get_data(int fd)
{
	static struct s_gnl_data	data = { 0, 0, 0 };
	size_t						i;
	
	if (!data.data && !cleanup)
	{
		data = (struct s_gnl_data){
			malloc(sizeof(struct s_gnl)), 1, 1
		};
		data.data[0] = (struct s_gnl) {fd, 0, 0};
		return (&data.data[0]);
	}
	i = 0;
	while (i < data.size)
	{
		if (data.data[i].fd == fd)
			return insert_or_remove(&data, fd, i, cleanup);
		++i;
	}
}

char *get_next_line(int fd)
{
	struct s_gnl	*data;
	ssize_t			nb_read;
	size_t			line_len;
	const void		*end;

	data = get_data(fd);
	while (1)
	{
		nb_read = read(fd, data->buffer, BUFFER_SIZE);
		end = __gnl_memnchr(data->buffer, '\n', nb_read);
		if (end)
		{
			
		}
		else
		{
			line_len += nb_read;
			__gnl_realloc(data->line, data->line_size, line_len);
			__gnl
		}
	}
	return (data->line);
}

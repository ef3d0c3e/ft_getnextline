#ifndef GET_NEXT_LINE_H
#define GET_NEXT_LINE_H

#include <stddef.h>
#define BUFFER_SIZE 42

/**
 * @brief Per file descriptor data
 */
struct s_gnl
{
	/**
	 * @brief The file descriptor
	 */
	int		fd;
	/**
	 * @brief The user's line
	 */
	char	*line;
	/**
	 * @brief Usable size of the line, for realloc
	 */
	size_t	line_size;
	/**
	 * @brief GNL internal buffer
	 */
	char	buffer[BUFFER_SIZE];
};

struct s_gnl_data
{
	s_gnl	*data;
	size_t	size;
	size_t	capacity;

};

char *get_next_line(int fd);

#endif // GET_NEXT_LINE_H

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgamba <marvin@student.42perpignan.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 11:54:01 by lgamba            #+#    #+#             */
/*   Updated: 2024/11/05 17:50:12 by lgamba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include <stddef.h>
# include <stdio.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1024
# endif // BUFFER_SIZE

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
	 * @brief Used size (in bytes) of the line
	 */
	size_t	line_sz;
	/**
	 * @brief Capacity (in bytes) of the line
	 */
	size_t	line_cap;
	/**
	 * @brief The user's line
	 */
	char	*line;
	/**
	 * @brief Number of bytes read in the last read() call
	 *
	 * Should never exceed @ref BUFFER_SIZE
	 */
	ssize_t	nb_read;
	/**
	 * @brief Position in the read buffer
	 */
	size_t	buf_pos;
	/**
	 * @brief Whether this file needs cleanup
	 */
	int		need_clean;
	/**
	 * @brief GNL internal buffer
	 *
	 * @note `BUFFER_SIZE` is controlled via a macro passed directly to the
	 * compiler, e.g: `-DBUFFER_SIZE=50`
	 */
	char	buffer[BUFFER_SIZE];
};

/**
 * @brief Global GNL data
 */
struct s_gnl_data
{
	/**
	 * All used GNL files
	 */
	struct s_gnl	*data;
	/**
	 * @brief The number of GNL files
	 */
	size_t			size;
	/**
	 * @brief Internal buffer capacity
	 */
	size_t			capacity;

};

/**
 * @brief GNL entry point
 *
 * The first call to this function with a new file descriptor will allocate a
 * @ref s_gnl structure and store it in the global @ref s_gnl_data.
 * Subsequent calls to this function will return the next line of the file.
 * When the end of the file is reached, a last call to @ref get_next_line is
 * expected to perform cleanup. Upon cleanup, all heap data will be freed.
 *
 * @param fd The file descriptor to read line by line
 *
 * @returns The currently read line in the file descriptor
 *
 * @note This function does not reset the cursor in the file descriptor, expect
 * undefined behaviour if you have read from the file descriptor before/inbetw-
 * een calls to this fuction.
 *
 * @warning This function is not thread safe! Because of a global variable
 * holding the currently open file descriptors, expect data races when using
 * @ref get_next_line from multiple threads.
 */
char	*get_next_line(int fd);

#endif // GET_NEXT_LINE_H

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lgamba <marvin@student.42perpignan.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 11:54:01 by lgamba            #+#    #+#             */
/*   Updated: 2024/11/05 17:50:12 by lgamba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef GNL_H
# define GNL_H

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
char				*get_next_line(int fd);

#endif // GNL_H

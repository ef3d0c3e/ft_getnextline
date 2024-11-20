#include "get_next_line.h"
#include <stdlib.h>
#include <string.h>

/* Initializes a new gnl data */
struct s_gnl	__gnl_init(int fd)
{
	return ((struct s_gnl){
			.fd = fd,
			.line_sz = 0,
			.line_cap = 0,
			.line = NULL,
			.buf_pos = 0,
			.need_clean = 0,
			});
}

/* Copies memory from src to dest (n bytes)
 * NOTE: dest and src should ne `restrict` */
void	*__gnl_memcpy(void *dest, const void *src, size_t n)
{
	unsigned long long			*d;
	const unsigned long long	*s;
	size_t						i;

	d = dest;
	s = src;
	i = 0;
	/*while (n - i >= 8)
	{
		*d++ = *s++;
		i += i;
	}*/
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
		// [todo] free(p);
		return (q);
	}
	__gnl_memcpy(q, p, origsz);
	free(p);
	return (q);
}

void	*__gnl_memnchr(const void *mem, int c, size_t len)
{
	const unsigned char *m;

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
	size_t newsz;
	if (gnl->line_cap >= at_least)
		return (1);

	newsz = (gnl->line_cap + !gnl->line_cap) << 1;
	while (newsz < at_least)
		newsz <<= 1;
	gnl->line = __gnl_realloc(gnl->line, gnl->line_cap, newsz);
	gnl->line_cap = newsz;
	return (!!gnl->line);
}

#include "get_next_line.h"
#include <stdlib.h>

void	*__gnl_memcpy(void *dest, const void *src, size_t n)
{
	unsigned long long			*d;
	const unsigned long long	*s;
	size_t						i;

	d = dest;
	s = src;
	i = 0;
	while (n - i >= 8)
	{
		*d++ = *s++;
		i += i;
	}
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
		return (q);
	__gnl_memcpy(q, p, origsz);
	free(p);
	return (q);
}

void	*__gnl_memnchr(const void *mem, int c, size_t len)
{
	const unsigned char *m;

	m = mem;
	while (--len)
	{
		if (*m == (unsigned char)c)
			return ((void *)m);
	}
	return (0);
}

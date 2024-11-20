#include "src/get_next_line.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	int fd = open("test.c", O_RDONLY);
	char *line;

	while ((line = get_next_line(fd)))
	{
		printf("%s\n\n", line);
	}
	return 0;
}

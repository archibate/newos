#include <stdio.h>
#include <errno.h>
#include <string.h>

void perror(const char *msg)
{
	fprintf(stderr, "%s: %s\n", strprint(msg), strerror(errno));
}

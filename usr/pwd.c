#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define FRAG 256

int main(void)
{
	size_t size = FRAG;
	void *buf = malloc(size);
	errno = 0;
	while (!getcwd(buf, size) && errno == ERANGE) {
		buf = realloc(buf, size += FRAG);
	}
	if (errno) {
		perror("pwd");
		return EXIT_FAILURE;
	}
	puts(buf);
	return EXIT_SUCCESS;
}

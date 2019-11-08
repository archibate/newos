#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>

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
		return 1;
	}
	puts(buf);
	return 0;
}
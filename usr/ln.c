#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "ln: missing oprand\n");
		return 1;
	}
	if (link(argv[1], argv[2]) == -1) {
		fprintf(stderr, "ln %s -> %s: %s\n",
				argv[1], argv[2], strerror(errno));
		return 1;
	}
	return 0;
}

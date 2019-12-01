#include "busybox.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "bkg: missing oprand\n");
		return 1;
	}
	argv++;
	argc--;
	pid_t pid = fork();
	if (pid < 0) {
		perror("cannot fork to background");
		return 1;
	}
	if (pid > 0) {
		printf("[%d] %s\n", pid, argv[0]);
		return 0;
	}
	execvp(argv[0], argv);
	perror(argv[0]);
	return 1;
}

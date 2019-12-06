#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static int do_kill(const char *proc, int sig)
{
	if (-1 == kill(atoi(proc), sig)) {
		perror(proc);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int sig = SIGINT;
	if (argc > 1 && argv[1][0] == '-') {
		sig = atoi(argv[1] + 1);
		argv++;
		argc--;
	}
	if (argc <= 1) {
		fprintf(stderr, "kill: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = do_kill(argv[i], sig) || err;
	return err;
}

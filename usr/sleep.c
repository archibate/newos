#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int do_sleep(const char *itstr)
{
	return sleep(atoi(itstr));
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "sleep: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = do_sleep(argv[i]) || err;
	return err;
}

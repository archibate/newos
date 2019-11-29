#include "busybox.h"
#include <stdio.h>
#include <libgen.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "dirname: missing oprand\n");
		return 1;
	}
	for (int i = 1; i < argc; i++)
		puts(dirname(argv[i]));
	return 0;
}

#include "busybox.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "copy.h"

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "cp: missing oprand\n");
		return 1;
	}
	return do_copy(argv[1], argv[2]);
}

#include "busybox.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "copy.h"

static int do_move(const char *src, const char *dst)
{
	if (link(src, dst) == -1) {
		if (do_copy(src, dst))
			return 1;
	}
	if (unlink(src) == -1) {
		fprintf(stderr, "unlink: %s: %s\n", src, strerror(errno));
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "mv: missing oprand\n");
		return 1;
	}
	return do_move(argv[1], argv[2]);
}

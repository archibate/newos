#include "busybox.h"
#include <stdio.h>
#include <string.h>

static int cat(const char *path)
{
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");
	if (!fp) {
		perror(path);
		return 1;
	}

	int c;
	while (EOF != (c = getc(fp)))
		putc(c, stdout);

	if (fp != stdin) fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	if (argc <= 1) err = cat("-");
	else for (int i = 1; i < argc; i++)
		err = cat(argv[i]) || err;
	return err;
}

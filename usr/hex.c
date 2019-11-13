#include <stdio.h>
#include <string.h>

int hex(const char *path)
{
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");
	if (!fp) {
		perror(path);
		return 1;
	}

	int c;
	static int i;
	while (EOF != (c = getc(fp))) {
		printf("%02X", (unsigned)c & 0xff);
		putchar(" \n"[++i % 26 == 0]);
	}
	if (i % 26 != 0)
		putchar('\n');

	if (fp != stdin) fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	if (argc <= 1) err = hex("-");
	else for (int i = 1; i < argc; i++)
		err = hex(argv[i]) || err;
	return err;
}

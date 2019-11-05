#include <stdio.h>
#include <string.h>

void cat(const char *path)
{
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");
	if (!fp) {
		perror(path);
		return;
	}

	char buf[256];
	size_t size;
	while ((size = fread(buf, 1, sizeof(buf), fp)))
		fwrite(buf, size, 1, stdout);

	if (fp != stdin) fclose(fp);
}

int main(int argc, char **argv)
{
	if (argc == 1) cat("-");
	else for (int i = 1; i < argc; i++)
		cat(argv[i]);
	return 0;
}

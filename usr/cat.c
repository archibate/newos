#include <stdio.h>
#include <string.h>

int cat(const char *path)
{
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");
	if (!fp) {
		perror(path);
		return 1;
	}

	char buf[256];
	while (fgets(buf, sizeof(buf), fp))
		fputs(buf, stdout);

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

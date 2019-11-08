#include <stdio.h>
#include <unistd.h>

int rm(const char *path)
{
	if (unlink(path) == -1) {
		perror(path);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "rm: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = err || rm(argv[i]);
	return err;
}

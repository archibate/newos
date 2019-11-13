#include <stdio.h>
#include <unistd.h>

int do_rmdir(const char *path)
{
	if (rmdir(path) == -1) {
		perror(path);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "rmdir: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = do_rmdir(argv[i]) || err;
	return err;
}

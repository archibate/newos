#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int do_mkdir(const char *path)
{
	if (mkdir(path, 0755) == -1) {
		perror(path);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	if (argc <= 1) fprintf(stderr, "mkdir: missing oprand\n");
	else for (int i = 1; i < argc; i++)
		err = do_mkdir(argv[i]) || err;
	return err;
}

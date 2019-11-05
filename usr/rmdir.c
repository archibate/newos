#include <stdio.h>
#include <unistd.h>

void do_rmdir(const char *path)
{
	if (rmdir(path) == -1)
		perror(path);
}

int main(int argc, char **argv)
{
	if (argc == 1) fprintf(stderr, "rmdir: missing oprand\n");
	else for (int i = 1; i < argc; i++)
		do_rmdir(argv[i]);
	return 0;
}

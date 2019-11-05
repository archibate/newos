#include <stdio.h>
#include <unistd.h>

void rm(const char *path)
{
	if (unlink(path) == -1)
		perror(path);
}

int main(int argc, char **argv)
{
	if (argc == 1) fprintf(stderr, "rm: missing oprand\n");
	else for (int i = 1; i < argc; i++)
		rm(argv[i]);
	return 0;
}

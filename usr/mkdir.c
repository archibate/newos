#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

void do_mkdir(const char *path)
{
	if (mkdir(path, 0755) == -1)
		perror(path);
}

int main(int argc, char **argv)
{
	if (argc == 1) fprintf(stderr, "mkdir: missing oprand\n");
	else for (int i = 1; i < argc; i++)
		do_mkdir(argv[i]);
	return 0;
}

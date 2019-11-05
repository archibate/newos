#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

void touch(const char *path)
{
	int fd = open(path, O_CREAT | O_EXCL, 0644 | S_IFREG);
	if (fd >= 0) close(fd);
	else if (errno != EEXIST) perror(path);
}

int main(int argc, char **argv)
{
	if (argc == 1) fprintf(stderr, "touch: missing oprand\n");
	else for (int i = 1; i < argc; i++)
		touch(argv[i]);
	return 0;
}

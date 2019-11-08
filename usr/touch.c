#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int touch(const char *path)
{
	int fd = open(path, O_CREAT | O_EXCL, 0644 | S_IFREG);
	if (fd >= 0) close(fd);
	else if (errno != EEXIST) {
		perror(path);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "touch: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = err || touch(argv[i]);
	return err;
}

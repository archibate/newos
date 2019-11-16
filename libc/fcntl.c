#include <fcntl.h>
#include <unistd.h>

int creat(const char *path, mode_t mode)
{
	return open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int dup(int fd)
{
	return fcntl(fd, F_DUPFD, 0);
}

int dup2(int fd, int fd2)
{
	if (fd == fd2)
		return fd2;
	close(fd2);
	return fcntl(fd, F_DUPFD, fd2);
}

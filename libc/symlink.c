#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int symlink(const char *path1, const char *path2)
{
	int fd = open(path2, O_WRONLY | O_CREAT | O_EXCL | O_SYMLINK, S_IFLNK | 0777);
	if (fd == -1)
		return -1;
	write(fd, path1, strlen(path1));
	close(fd);
	return 0;
}

ssize_t readlink(const char *path, char *buf, size_t size)
{
	int fd = open(path, O_RDONLY | O_SYMLINK);
	if (fd == -1)
		return -1;
	ssize_t ret = read(fd, buf, size);
	close(fd);
	return ret;
}

ssize_t readlinkat(int fd, const char *path, char *buf, size_t size)
{
	fd = openat(fd, path, O_RDONLY | O_SYMLINK);
	if (fd == -1)
		return -1;
	ssize_t ret = read(fd, buf, size);
	close(fd);
	return ret;
}

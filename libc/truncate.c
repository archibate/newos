#include <fcntl.h>
#include <unistd.h>

int truncate(const char *path, off_t length)
{
	int fd = open(path, O_WRONLY);
	if (fd == -1)
		return -1;
	int ret = ftruncate(fd, length);
	close(fd);
	return ret;
}

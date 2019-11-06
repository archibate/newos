#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int stat(const char *path, struct stat *st)
{
	int fd = open(path, 0);
	if (fd < 0 && errno == EISDIR) fd = open(path, O_DIRECTORY);
	if (fd < 0)
		return -1;
	int ret = fstat(fd, st);
	close(fd);
	return ret;
}

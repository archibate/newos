#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int ftruncate_s(int fd, size_t length);
static int ftruncate_x(int fd, size_t length)
{
	size_t m;
	ssize_t ret;
	off_t old_pos = lseek(fd, SEEK_CUR, 0);
	if (old_pos == -1)
		return -1;
	lseek(fd, SEEK_END, 0);
	static char buf[2048];
	while (length > 0) {
		m = length;
		if (m > sizeof(buf))
			m = sizeof(buf);
		ret = write(fd, buf, m);
		if (ret == -1)
			return -1;
		length -= ret;
	}
	lseek(fd, SEEK_SET, old_pos);
	return 0;
}

int ftruncate(int fd, off_t length)
{
	if (length < 0) {
		errno = EINVAL;
		return -1;
	}
	int ret = ftruncate_s(fd, length);
	if (ret == -1)
		return -1;
	if (ret == 0)
		return 0;
	return ftruncate_x(fd, length);
}

int truncate(const char *path, off_t length)
{
	int fd = open(path, O_WRONLY | O_CREAT, 0644 | S_IFREG);
	if (fd == -1)
		return -1;
	int ret = ftruncate(fd, length);
	close(fd);
	return ret;
}

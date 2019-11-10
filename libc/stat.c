#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int stat(const char *path, struct stat *st)
{
	return fstatat(AT_FDCWD, path, st, 0);
}

int lstat(const char *path, struct stat *st)
{
	return fstatat(AT_FDCWD, path, st, AT_SYMLINK_NOFOLLOW);
}

int fstat(int fd, struct stat *st)
{
	return fstatat(fd, "", st, AT_EMPTY_PATH);
}

#include <unistd.h>
#include <fcntl.h>

int unlink(const char *path)
{
	return unlinkat(AT_FDCWD, path, 0);
}

int rmdir(const char *path)
{
	return unlinkat(AT_FDCWD, path, AT_REMOVEDIR);
}

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

int remove(const char *path)
{
	if (unlinkat(AT_FDCWD, path, AT_REMOVEDIR) == -1)
		return unlinkat(AT_FDCWD, path, 0);
	return 0;
}

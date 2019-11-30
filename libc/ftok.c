#include <sys/ipc.h>
#include <sys/stat.h>

key_t ftok(const char *path, int proj_id)
{
	key_t key;
	struct stat st;
	if (-1 == lstat(path, &st))
		return -1;
	key = st.st_dev & 0xff;
	key <<= 16;
	key |= st.st_ino & 0xffff;
	key <<= 8;
	key |= proj_id & 0xff;
	return key;
}

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

char *getcwd(char *buf, size_t size)
{
	struct stat st;
	struct dirent de;
	char tmpbuf[size];
	int fd, tmp;
	if (!size--)
		return NULL;
	buf[0] = 0;
	fd = open(".", O_RDONLY | O_DIRECTORY);
	if (fd == -1)
		return NULL;
again:
	if (fstat(fd, &st) == -1)
		return NULL;
	tmp = openat(fd, "..", O_RDONLY | O_DIRECTORY);
	close(fd);
	fd = tmp;
	if (fd == -1)
		return NULL;

	dirread(fd, &de); // 1st must be "."
	if (st.st_ino == de.d_ino) {
		if (!buf[0])
			strcpy(buf, "/");
		close(fd);
		return buf;
	}
	dirread(fd, &de); // 2nd must be ".."

	while (-1 != dirread(fd, &de)) {
		if (de.d_ino == st.st_ino) {
			strcpy(tmpbuf, buf);
			strcpy(buf, "/");
			strcat(buf, de.d_name);
			if (size < strlen(buf)) {
				errno = ERANGE;
				goto bad;
			}
			size -= strlen(buf);
			strcat(buf, tmpbuf);
			goto again;
		}
	}

	errno = ENOENT;
bad:
	close(fd);
	return NULL;
}

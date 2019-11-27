#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "strstat.h"

int ls_stat(int fd, const char *path)
{
	struct stat st;
	if (fstatat(fd, path, &st, AT_SYMLINK_NOFOLLOW) == -1) {
		perror(path);
		return 1;
	}
	printf("%9s %5ld %s\n", strfaccess(st.st_mode), st.st_size, path);
	return 0;
}

int ls(const char *path)
{
	int i, fd = open(path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) {
		if (errno == ENOTDIR)
			return ls_stat(AT_FDCWD, path);
		perror(path);
		return 1;
	}
	struct dirent de;
	while (-1 != (i = dirread(fd, &de))) {
		if (!i) continue;
		if (ls_stat(fd, de.d_name))
			printf("?????????? ????? %s\n", de.d_name);
	}
	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	if (argc <= 1) err = ls(".");
	else if (argc == 2) err = ls(argv[1]);
	else for (int i = 1; i < argc; i++) {
		if (i != 1) putchar('\n');
		printf("%s:\n", argv[i]);
		err = ls(argv[i]) || err;
	}
	return err;
}

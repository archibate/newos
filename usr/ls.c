#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

void ls_stat(const char *path)
{
	struct stat st;
	if (stat(path, &st) == -1) {
		perror(path);
		return;
	}
	printf("%7d %s\n", st.st_ino, path);
}

void ls(const char *path)
{
	int i, fd = open(path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) {
		if (errno == ENOTDIR)
			return ls_stat(path);
		perror(path);
		return;
	}
	struct dirent de;
	while (-1 != (i = dirread(fd, &de))) {
		if (i) printf("%7d %s\n", de.d_ino, de.d_name);
	}
	close(fd);
}

int main(int argc, char **argv)
{
	if (argc == 1) ls(".");
	else if (argc == 2) ls(argv[1]);
	else for (int i = 1; i < argc; i++) {
		if (i != 1) putchar('\n');
		printf("%s:\n", argv[i]);
		ls(argv[i]);
	}
	return 0;
}

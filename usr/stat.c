#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "strstat.h"

char *strftype(mode_t mode)
{
	if (S_ISREG(mode))
		return "regular file";
	if (S_ISDIR(mode))
		return "directory";
	if (S_ISLNK(mode))
		return "symbolic link";
	if (S_ISFIFO(mode))
		return "fifo";
	if (S_ISCHR(mode))
		return "character device";
	if (S_ISBLK(mode))
		return "block device";
	if (S_ISSOCK(mode))
		return "socket";
	static char buf[23];
	sprintf(buf, "unknown (%#06o)", mode);
	return buf;
}

int do_stat(const char *path)
{
	struct stat st;
	if (lstat(path, &st) == -1) {
		perror(path);
		return 1;
	}
	printf("  File: %s\n",    path);
	printf("  Type: %s\n",    strftype(st.st_mode));
	printf("  Size: %ld\n",   st.st_size);
	printf(" Inode: %d\n",    st.st_ino);
	printf(" Links: %d\n",    st.st_nlink);
	printf("Access: %#03o (%s)\n", st.st_mode & 0777, strfaccess(st.st_mode));
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "stat: missing oprand\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = err || do_stat(argv[i]);
	return err;
}

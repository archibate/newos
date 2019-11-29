#include <stdio.h>
#include <sys/stat.h>

static int chrftype(mode_t mode)
{
	if (S_ISREG(mode))
		return '-';
	if (S_ISDIR(mode))
		return 'd';
	if (S_ISLNK(mode))
		return 'l';
	if (S_ISFIFO(mode))
		return 'p';
	if (S_ISCHR(mode))
		return 'c';
	if (S_ISBLK(mode))
		return 'b';
	if (S_ISSOCK(mode))
		return 'n';
	return '?';
}

static char *strfaccess(mode_t mode)
{
	int i;
	static char buf[11] = "?rwxrwxrwx";
	buf[0] = chrftype(mode);
	for (int i = 0; i < 9; i++) {
		if (!(mode & 1 << (8 - i)))
			buf[1 + i] = '-';
	}
	return buf;
}

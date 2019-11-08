#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int do_stat(const char *path)
{
	struct stat st;
	if (stat(path, &st) == -1) {
		perror(path);
		return 1;
	}
	printf(" File: %s\n",    path);
	printf(" Size: %ld\n",   st.st_size);
	printf("Inode: %d\n",    st.st_ino);
	printf("Links: %d\n",    st.st_nlink);
	printf(" Mode: %#06o\n", st.st_mode);
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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int do_copy(const char *src, const char *dst)
{
	FILE *fin = fopen(src, "r");
	if (!fin) {
		perror(src);
		return -1;
	}
	struct stat st;
	fstat(fileno(fin), &st);
	int fd = open(dst, O_CREAT | O_WRONLY | O_TRUNC, st.st_mode);
	FILE *fout = fdopen(fd, "w");
	if (!fout) {
		perror(dst);
		close(fd);
		fclose(fin);
		return -1;
	}
	char buf[256];
	size_t size;
	while ((size = fread(buf, 1, sizeof(buf), fin)))
		fwrite(buf, size, 1, fout);
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int do_copy(const char *src, const char *dst)
{
	// TODO: cannot copy attrs when dst already exist. use unlink(dst) plz.
	FILE *fin = fopen(src, "r");
	if (!fin) {
		perror(src);
		return 1;
	}
	struct stat st;
	fstat(fileno(fin), &st);
	int fd = open(dst, O_CREAT | O_WRONLY | O_TRUNC, st.st_mode);
	if (fd == -1) goto err;
	FILE *fout = fdopen(fd, "w");
	if (!fout) {
		close(fd);
err:
		perror(dst);
		fclose(fin);
		return 1;
	}
	char buf[256];
	size_t size;
	while ((size = fread(buf, 1, sizeof(buf), fin)))
		fwrite(buf, size, 1, fout);
	return 0;
}

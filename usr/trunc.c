#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

size_t my_atoi(const char *s)
{
	size_t r = strtol(s, (char **)&s, 10);
	switch (*s) {
	case 'K': return r << 10;
	case 'M': return r << 20;
	case 'G': return r << 30;
	default:  return r;
	}
}

size_t get_size(const char *sz, size_t size)
{
	if ('0' <= sz[0] && sz[0] <= '9')
		return my_atoi(sz);
	size_t x = my_atoi(sz + 1);
	switch (sz[0]) {
	case '+':
		return size + x;
	case '-':
		return size - x;
	case '>':
		return size > x ? size : x;
	case '<':
		return size < x ? size : x;
	case '/':
		return size / x * x;
	case '%':
		return (size + x - 1) / x * x;
	default:
		return my_atoi(sz);
	}
}

int do_truncate(const char *sz, const char *path)
{
	struct stat st;
	stat(path, &st);
	size_t size = get_size(sz, st.st_size);
	if (size == (size_t)st.st_size)
		return 0;
	if (truncate(path, size) == -1) {
		perror(path);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "trunc: missing oprand\n");
		return 0;
	}
	return do_truncate(argv[1], argv[2]);
}

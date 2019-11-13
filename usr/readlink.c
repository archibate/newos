#include <stdio.h>
#include <unistd.h>
#include <string.h>

int do_readlink(const char *path)
{
	char buf[1025];
	ssize_t ret = readlink(path, buf, sizeof(buf) - 1);
	if (ret == -1)
		return -1;
	buf[ret] = 0;
	puts(buf);
	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	if (argc <= 1) {
		fprintf(stderr, "readlink: missing oprand\n");
		return 1;
	}
	for (int i = 1; i < argc; i++)
		err = do_readlink(argv[i]) || err;
	return err;
}

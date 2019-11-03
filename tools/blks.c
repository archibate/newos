#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#include <stdio.h>
#include <stdlib.h>

#define BSIZE 1024

int main(int argc, char **argv)
{
	if (!argc) {
		fprintf(stderr, "no file specified\n");
		return EXIT_FAILURE;
	}
	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fclose(fp);

	printf("%d\n", (size + BSIZE - 1) / BSIZE);

	return EXIT_SUCCESS;
}

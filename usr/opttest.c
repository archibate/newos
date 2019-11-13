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
#include <unistd.h>

int main(int argc, char **argv)
{
	int c;
	while (-1 != (c = getopt(argc, argv, "-abcf:"))) {
		printf("optind: %d\n", optind);
		printf("optchr: %c\n", c);
		printf("optopt: %c\n", optopt);
		printf("optarg: %s\n", optarg);
	}
	return 0;
}

#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $* | sort -n
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

int tab[256];

int main(int argc, char **argv)
{
	int c, i;
	FILE *fp = stdin;
	if (argv[1]) {
		fp = fopen(argv[1], "r");
		if (!fp)
			perror(argv[1]);
	}
	while (!feof(fp)) {
		c = getc(fp);
		tab[c & 0xff]++;
	}
	for (i = 0; i < 256; i++) {
		if (tab[i])
			printf("%d\t%02x\n", tab[i], i);
	}
	if (fp != stdin)
		fclose(fp);
	return 0;
}

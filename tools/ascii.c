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
#include <signal.h>

void ex(void)
{
	system("stty icanon");
	system("stty echo");
}

int main(void)
{
	system("stty -echo");
	system("stty -icanon");
	atexit(ex);
	signal(SIGINT, exit);
	while (1) {
		unsigned char c = getchar();
		printf("%02X\n", c);
	}
}

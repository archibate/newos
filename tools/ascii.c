#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$ && /tmp/$$ $*
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
#include <string.h>
#include <signal.h>
#include <termios.h>

struct termios old_tc, new_tc;

void endscrn(void)
{
	tcsetattr(0, TCSANOW, &old_tc);
}

void initscrn(void)
{
	if (tcgetattr(0, &old_tc) == -1)
		return;
	memcpy(&new_tc, &old_tc, sizeof(struct termios));
	new_tc.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &new_tc);
	atexit(endscrn);
	signal(SIGINT, exit);
}

int main(void)
{
	initscrn();
	while (1) {
		unsigned char c = getchar();
		printf("%02X\n", c);
	}
}

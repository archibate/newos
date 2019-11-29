#if 0 ////////////////////////////////////////// {{{
true /*; chmod +x $0
if grep -q 'math\.h' $0; then C+=-lm; fi
if grep -q 'pthread\.h' $0; then C+=-lpthread; fi
if [ ! -z $GDB ]; then C+="-gstabs+ -ggdb -D_DEBUG"; fi
gcc $C -Werror $0 -o /tmp/$$ && $GDB /tmp/$$ $*; x=$?
rm -f /tmp/$$; exit $x
true CCSH_signature123 */
#endif
// # }}} [3J[H[2J
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

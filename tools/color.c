#if 0 // sh-stub {{{
true /*
chmod +x $0
if [ ! -z $GDB ]; then C="-gstabs+ -ggdb -D_DEBUG"; fi
gcc $C -Werror -D_ARGV0=\"$0\" $0 -o /tmp/$$ && $GDB /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#include <stdio.h>

int main(void)
{
	int i;
	for (i = 0; i < 7; i++)
		printf("\033[3%dm^[[3%dm\033[0m\n", i, i);
	for (i = 0; i < 7; i++)
		printf("\033[1;3%dm^[[1;3%dm\033[0m\n", i, i);
	for (i = 0; i < 7; i++)
		printf("\033[4%dm^[[4%dm\033[0m\n", i, i);
	return 0;
}

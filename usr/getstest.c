#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -ggdb -gstabs+ -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
#cgdb /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#include <stdio.h>
#include <string.h>

int main(void)
{
	char buf[2048];
	while (fgets(buf, sizeof(buf), stdin)) {
		fputs(buf, stdout);
	}
	return 0;
}

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
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void)
{
	pid_t pid = fork();
	int stat = 0;
	if (pid == 0) {
		printf("child\n");
	} else if (pid > 0) {
		printf("parent\n");
		for (volatile int i = 0; i < 23333333; i++);
		printf("parent done\n");
		if (waitpid(pid, &stat, 0) == -1)
			perror("waitpid");
	} else {
		perror("fork");
		stat = 1;
	}
	return stat;
}

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
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void)
{
	int fd[2];
	printf("making pipe...\n");
	if (pipe(fd) == -1) {
		perror("pipe");
		return 1;
	}
	printf("forking...\n");
	pid_t pid = fork();
	if (pid == 0) {
		printf("child\n");
		close(fd[0]);
		if (write(fd[1], "Hello, World!", 13) != 13)
			perror("write");
	} else {
		printf("parent\n");
		close(fd[1]);
		char buf[233];
		buf[read(fd[0], buf, sizeof(buf))] = 0;
		printf("parent got [%s] from child\n", buf);
		int stat;
		wait(&stat);
	}
	return 0;
}

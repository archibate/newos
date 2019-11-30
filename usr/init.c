#include "busybox.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int stat = 1;
	for (int i = 0; i < _NSIG; i++)
		signal(i, SIG_IGN);

	pid_t pid = fork();
	if (pid == 0) {
		execl("/bin/sh", "/bin/sh", "/etc/rcS", NULL);
		perror("/bin/sh");
	} else if (pid < 0) {
		perror("fork");
	} else {
		if (waitpid(pid, &stat, 0) == -1)
			perror("waitpid");
		stat = WEXITSTATUS(stat);
		fprintf(stderr, "init: /bin/sh exited with %#x\n", stat);
	}
	return stat;
}

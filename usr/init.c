#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void)
{
	int stat;
	for (int i = 0; i < _NSIG; i++)
		signal(i, SIG_IGN);

	pid_t pid = fork();
	if (pid == 0) {
		execl("/bin/sh", "/bin/sh", NULL);
		perror("/bin/sh");
	} else if (pid < 0) {
		perror("fork");
	} else {
		if (waitpid(pid, &stat, 0) == -1)
			perror("waitpid");
		fprintf(stderr, "/bin/sh exited with %#x\n", stat);
	}
	return stat;
}

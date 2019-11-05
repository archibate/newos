#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int system(const char *s)
{
	pid_t pid = fork();
	if (pid == 0) {
		return execlp("sh", "sh", "-c", s, NULL);
	} else if (pid > 0) {
		int stat;
		while (wait(&stat) != pid);
		return stat;
	} else {
		return pid;
	}
}

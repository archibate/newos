#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int system(const char *s)
{
	int stat;
	pid_t pid = fork();
	if (pid == 0) {
		return execlp("sh", "sh", "-c", s, NULL);
	} else if (pid > 0) {
		while (wait(&stat) != pid);
		return stat;
	} else {
		return pid;
	}
}

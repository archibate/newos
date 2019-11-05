#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	printf("Hello from user!\n");

	int stat;
	pid_t pid = fork();
	if (pid == 0) {
		printf("Hello from child!\n");
		for (int i = 0; i < 10000; i++);
		exit(233);
	} else {
		printf("Hello from parent!\n");
		waitpid(pid, &stat, 0);
		printf("Child exited with %d\n", WEXITSTATUS(stat));
	}
}

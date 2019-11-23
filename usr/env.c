#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
	if (argc <= 1) {
		while (*envp)
			puts(*envp++);
		return 0;
	}
	execvp(argv[1], argv + 1);
	perror(argv[1]);
	return 1;
}

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **_argv, char **envp)
{
	environ = envp;

	open("/dev/tty/mux", O_RDONLY);
	open("/dev/tty/mux", O_WRONLY);
	dup(1);

	write(1, "exectest...\n", 12);

	if (fork() == 0) {
		execlp("argvtest",
				"argvtest", "--test",
				"file.c", NULL);
		write(2, "exec error\n", 11);
	} else {
		write(1, "exectest's parent\n", 18);
	}

	for (;;)
		pause();
}

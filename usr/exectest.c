#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>

int main(int argc, char **_argv, char **envp)
{
	open("/dev/tty/mux", O_RDONLY);
	open("/dev/tty/mux", O_WRONLY);
	dup(1);

	write(1, "exectest...\n", 12);

	if (fork() == 0) {
		char *argv[] = {
			"/bin/argvtest",
			"--test", "file.c",
			NULL};
		execve(argv[0], argv, envp);
		write(1, "exec error\n", 11);
	}
	write(1, "exectest's parent\n", 18);

	for (;;)
		pause();
}

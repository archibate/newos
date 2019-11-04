#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv, char **envp)
{
	open("/dev/tty/mux", O_RDONLY);
	open("/dev/tty/mux", O_WRONLY);
	dup(1);

	write(1, "argv: ", 6);
	while (*argv) {
		write(1, *argv, strlen(*argv));
		write(1, *++argv ? " " : "\n", 1);
	}
	write(1, "envp:\n", 6);
	while (*envp) {
		write(1, *envp, strlen(*envp));
		write(1, "\n", 1);
		envp++;
	}

	for (;;)
		pause();
}

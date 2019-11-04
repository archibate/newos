#include <fcntl.h>
#include <unistd.h>

int main(char *const *argv)
{
	write(1, "Hello from user!\n", 17);
	if (fork() == 0) {
		write(1, "Hello from child!\n", 18);
	} else {
		write(1, "Hello from parent!\n", 19);
	}

	for (;;)
		pause();
}

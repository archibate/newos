#include <fcntl.h>
#include <unistd.h>

#define CONSOLE "/dev/tty/com0"

int main(void)
{
	open(CONSOLE, O_RDONLY);
	open(CONSOLE, O_WRONLY);
	dup(1);

	for (;;) {
		write(1, "Hello from user!\n", 17);
		pause();
	}
}

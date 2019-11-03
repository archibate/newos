#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	open("/dev/tty/mux", O_RDONLY);
	open("/dev/tty/mux", O_WRONLY);
	dup(1);

	for (;;) {
		write(1, "Hello from user!\n", 17);
		pause();
	}
}

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

int isatty(int fd)
{
	struct termios tc;
	return tcgetattr(fd, &tc) != -1;
}

int tcgetattr(int fd, struct termios *tc)
{
	return ioctl(fd, I_TC_GETATTR, tc);
}

int tcsetattr(int fd, int optop, const struct termios *tc)
{
	return ioctl(fd, I_TC_SETATTR, tc);
}

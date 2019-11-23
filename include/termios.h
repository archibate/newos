#ifndef _TERMIOS_H
#define _TERMIOS_H 1

#include <bits/termios.h>

int tcgetattr(int fd, struct termios *tc);
int tcsetattr(int fd, int optop, const struct termios *tc);

#endif

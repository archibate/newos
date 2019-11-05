#ifndef _BITS_WAIT_H
#define _BITS_WAIT_H 1

#include <bits/signal.h>

#define WNOHANG		1
#define WUNTRACED	2

#define WIFEXITED(w)	(((w) & 0xff) == 0)
#define WIFSIGNALED(w)	(((w) & 0xff) == 0xff)
#define WIFSTOPPED(w)	(((w) & 0xff) == 0x7f)
#define WEXITSTATUS(w)	(((w) >> 8) & 0xff)
#define WTERMSIG(w)	(((w) >> 16) & 0xff)
#define WSTOPSIG(w)	(SIGSTOP)

#endif

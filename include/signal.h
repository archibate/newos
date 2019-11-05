#ifndef _SIGNAL_H
#define _SIGNAL_H 1

#include <bits/signal.h>
#include <bits/types.h>

#ifndef pid_t
#define pid_t __pid_t
#endif

typedef void (*__sighandler_t)(int);

long sgetmask(void);
long ssetmask(long mask);
__sighandler_t signal(int sig, __sighandler_t handler);
__sighandler_t signal_r(int sig, __sighandler_t handler, void *restorer);
int kill(pid_t pid, int sig);
int raise(int sig);

#endif

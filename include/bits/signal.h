#ifndef _BITS_SIGNAL_H
#define _BITS_SIGNAL_H 1

#define SIGINT	2
#define SIGILL	4
#define SIGTRAP	5
#define SIGABRT	6
#define SIGFPE	8
#define SIGKILL	9
#define SIGUSR1	10
#define SIGSEGV	11
#define SIGUSR2	12
#define SIGPIPE	13
#define SIGALRM	14
#define SIGTERM	15
#define SIGCHLD	17
#define SIGSTOP	19
#define SIGPOLL	23
#define _NSIG	32

#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)
#define SIG_ERR ((void (*)(int))2)

#define SA_NOCLDSTOP	1
#define SA_NOMASK	0x40000000
#define SA_ONESHOT	0x80000000

typedef int sigset_t;

struct sigaction {
	void (*sa_handler)(int);
	void (*sa_restorer)(void);
	sigset_t sa_mask;
	int sa_flags;
};

#endif

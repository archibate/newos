#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

__attribute__((noreturn)) void abort(void)
{
	ssetmask(sgetmask() & ~(1 << (SIGABRT - 1)));
	raise(SIGABRT);
	signal(SIGABRT, SIG_DFL);
	raise(SIGABRT);
	_exit(EXIT_FAILURE);
}

__attribute__((noreturn)) void _Exit(int status)
{
	_exit(status);
}

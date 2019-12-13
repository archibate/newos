#include <signal.h>

int kill(pid_t pid, int sig)
{
	return kill_a(pid, sig, 0);
}

int raise(int sig)
{
	return raise_a(sig, 0);
}

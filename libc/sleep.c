#include <unistd.h>
#include <signal.h>

static int sleepok;

static void sigalrm(int sig)
{
	sleepok = 1;
}

unsigned sleep(unsigned secs)
{
	sleepok = 0;
	signal(SIGALRM, sigalrm);
	alarm(secs);
	pause();
	return sleepok ? 0 : secs;
}

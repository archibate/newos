#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void sigfpe(int sig)
{
	printf("caught SIGFPE\n");
	abort();
}

int main(void)
{
	signal(SIGFPE, sigfpe);
	printf("testing 1/0...\n");
	int x = 0;
	x = 1 / x;
	printf("testing 1/0 done!!!\n");
	return 0;
}

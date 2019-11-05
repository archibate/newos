#include <sys/wait.h>

pid_t wait(int *stat_loc)
{
	return waitpid(-1, stat_loc, 0);
}

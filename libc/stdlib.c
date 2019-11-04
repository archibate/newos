#include <stdlib.h>
#include <unistd.h>

__attribute__((noreturn)) void abort(void)
{
	exit(EXIT_FAILURE);
}

__attribute__((noreturn)) void _Exit(int status)
{
	_exit(status);
}

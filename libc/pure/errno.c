#include <errno.h>

static int _errno;

int *__errno_location(void)
{
	return &_errno;
}

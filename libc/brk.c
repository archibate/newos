#include <unistd.h>

int brk(void *addr)
{
	void *p = sbrk(0);
	if (p == (void *)-1)
		return -1;
	p = sbrk(addr - p);
	if (p == (void *)-1)
		return -1;
	return 0;
}

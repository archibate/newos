#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -ggdb -gstabs+ -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
#cgdb /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(void)
{
	void *p = (void *)0x80000000;
	int fd = open("m", O_RDWR);
	if (fd == -1) {
		perror("m");
		return 1;
	}
	printf("performing mmap...\n");
	if (MAP_FAILED == mmap(p, 8192, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_FIXED, fd, 0)) {
		perror("mmap");
		return 1;
	}
	close(fd);
	printf("data at mapped area + 0: [%.20s]\n", (char *)p);
	*(char *)(p + 4098) = 'a';
	printf("data at mapped area + 4098: [%.20s]\n", (char *)p + 4098);
	munmap(p, 4096);
	printf("data at mapped area + 4098: [%.20s]\n", (char *)p + 4098);
	printf("data at mapped area + 0 (should fail): [%.20s]\n", (char *)p);
	return 0;
}

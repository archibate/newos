#ifndef _BUSYBOX_H
#define _BUSYBOX_H

struct busybox_entry
{
	const char *name;
	unsigned int namelen;
	int (*entry)(int, char **);
};

#ifndef _BUSYBOX_C
static int main(int argc, char **argv);
static char __this_file__[] = __BASE_FILE__;
__attribute__((section(".busybox"), weak))
struct busybox_entry __busybox_this__ =
{
	// skip "usr/":
	.name = __this_file__ + 4,
	// trim ".c\0":
	.namelen = sizeof(__this_file__) - 4 - 3,
	.entry = main,
};
#endif

#endif

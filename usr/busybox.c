#define _BUSYBOX_C
#include "busybox.h"
#include <stdio.h>
#include <string.h>
#include <libgen.h>

extern struct busybox_entry __BBOX_BEGIN__[0], __BBOX_END__[0];

int main(int argc, char **argv)
{
	struct busybox_entry *p;
	int i, (*entry)(int, char **) = NULL;
	argv[0] = basename(argv[0]);
	if (argc >= 1 && !strcmp(argv[0], "busybox")) {
		argv++;
		argc--;
	}
	if (argc <= 0) {
		for (p = __BBOX_BEGIN__; p < __BBOX_END__; p++)
			fprintf(stderr, "%.*s\n", p->namelen, p->name);
		return 1;
	}
	for (p = __BBOX_BEGIN__; p < __BBOX_END__; p++) {
		if (strlen(argv[0]) == p->namelen &&
			!memcmp(argv[0], p->name, p->namelen)) {
			entry = p->entry;
			break;
		}
	}
	if (!entry) {
		fprintf(stderr, "busybox: %s: command not found\n",
				strescape(argv[0]));
		return 1;
	}
	return entry(argc, argv);
}

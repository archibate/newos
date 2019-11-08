#ifndef _DS_UNITEST_H
#define _DS_UNITEST_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FAILED "[\033[1;31mFAILED\033[0m]"
#define   OK   "[\033[1;32m  OK  \033[0m]"
#define FORMAT "[      ] \033[1;33m%s\033[1;35m %s\033[0m"
#define TBLK do
#define ENDT while (0)
#define tprintf(...) fprintf(stderr, __VA_ARGS__)
#define T(x, cond) if (1) { \
	tprintf(FORMAT, #x, #cond); \
	if (!((x) cond)) { \
		int __err = errno; \
		tprintf("\r"FAILED"\n"); \
		tprintf("%s\n", strerror(__err)); \
		break; \
	} else { \
		tprintf("\r"OK"\n"); \
	} \
} else {}
#define TS(x, cond) TBLK T(x, cond) ENDT

#endif

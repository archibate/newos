#ifndef _STDLIB_H
#define _STDLIB_H 1

// Get NULL, size_t, etc.
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

extern char **environ;

char *getenv(const char *name);

#endif

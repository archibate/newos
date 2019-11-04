#ifndef _STDLIB_H
#define _STDLIB_H 1

// Get NULL, size_t, etc.
#include <stddef.h>
// Get malloc, free, etc.
#include <malloc.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 32767

extern char **environ;

typedef struct _div {
	int quot, rem;
} div_t;

char *getenv(const char *name);
__attribute__((noreturn)) void exit(int status);
__attribute__((noreturn)) void _Exit(int status);
__attribute__((noreturn)) void abort(void);

int rand_r(unsigned int *sp);
void srand(unsigned int s);
int rand(void);

long strtol(const char *s, char **endptr, int base);
int atoi(const char *s);
div_t div(int a, int b);
int abs(int x);

#endif

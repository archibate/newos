#ifndef _MALLOC_H
#define _MALLOC_H 1

// Get size_t.
#include <stddef.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *p, size_t size);
void free(void *p);

#endif

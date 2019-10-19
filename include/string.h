#ifndef _STRING_H
#define _STRING_H 1

// Get size_t, NULL from stddef.h
#include <stddef.h>

int strlen(const char *s);
int strnlen(const char *s, size_t size);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t size);
size_t strlcpy(char *dst, const char *src, size_t size);
int strcmp(const char *p, const char *q);
int strncmp(const char *p, const char *q, size_t n);
char *strchr(const char *s, int c);
char *strfind(const char *s, int c);
void *memset(void *v, int c, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *v1, const void *v2, size_t n);
void *memfind(const void *s, int c, size_t n);
long strtol(const char *s, char **endptr, int base);

#endif

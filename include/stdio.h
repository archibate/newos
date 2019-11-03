#ifndef _STDIO_H
#define _STDIO_H 1

// Get size_t, NULL from stddef.h
#include <stddef.h>
// Get va_list.
#include <stdarg.h>
// Get SEEK_*
#include <bits/unistd.h>

typedef struct _io_buf FILE;

extern FILE *stdin, *stdout, *stderr;

int printf(const char *fmt, ...);
int fprintf(FILE *fp, const char *fmt, ...);
int dprintf(int fd, const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);
int snprintf(char *str, size_t n, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int vfprintf(FILE *fp, const char *fmt, va_list ap);
int vdprintf(int fd, const char *fmt, va_list ap);
int vsprintf(char *str, const char *fmt, va_list ap);
int vsnprintf(char *str, size_t n, const char *fmt, va_list ap);

#endif

#ifndef _PRINTFMT_H
#define _PRINTFMT_H 1

// Get va_list.
#include <stdarg.h>

void printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt,
		va_list ap);

#endif

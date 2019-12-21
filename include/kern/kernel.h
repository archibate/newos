#ifndef _KERN_KERNEL_H
#define _KERN_KERNEL_H 1

// Get va_list.
#include <stdarg.h>

int printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list ap);
__attribute__((noreturn)) void do_panic(void);

#define panic(...) do { \
	printk("PANIC: " __VA_ARGS__); \
	do_panic(); \
} while (0)

#define assert(x) do { \
	if (!(x)) { \
		panic("assert(" #x ") failed"); \
	} \
} while (0)

#endif

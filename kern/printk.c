#include <kern/kernel.h>
#include <kern/tty.h>
#include <string.h>
#include <stdio.h>

int
printk(const char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	int ret = vprintk(fmt, ap);
	va_end(ap);
	return ret;
}

int
vprintk(const char *fmt, va_list ap)
{
	static char buf[1024];
	unsigned int tick;
	asm volatile ("rdtsc" : "=a" (tick));
	snprintf(buf, 15, "[%10ui] ", tick);
	int ret = vsnprintf(buf + 14, sizeof(buf) - 1, fmt, ap);
	size_t len = strnlen(buf + 14, sizeof(buf) - 2) + 14;
	buf[len++] = '\n';
	buf[len] = 0;
	tty_write(TTY_VGA, buf, len);
	tty_write(TTY_COM0, buf, len);
	return ret;
}

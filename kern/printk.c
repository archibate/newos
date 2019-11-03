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
	char buf[1024];
	int ret = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	size_t len = strnlen(buf, sizeof(buf) - 2);
	buf[len++] = '\n';
	tty_write(TTY_MUX, buf, len);
	return ret;
}

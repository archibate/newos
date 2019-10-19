#ifndef _KERN_TTY_H
#define _KERN_TTY_H 1

// Get size_t.
#include <stddef.h>
#include <kern/ringbuf.h>

#define TTY_VGA  0
#define TTY_COM0 1
#define NTTYS    2

#define TTY_BUFSIZ 512

typedef RINGBUF(char, TTY_BUFSIZ) tty_queue_t;

struct tty_struct
{
	int (*putc)(int);
	int (*getc)(int *);
	tty_queue_t read_q;
};

void tty_intr(int num);
size_t tty_read(int num, char *buf, size_t n);
void tty_write(int num, const char *buf, size_t n);
void tty_register(int num, int (*putc)(int), int (*getc)(int *));

#endif

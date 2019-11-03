#ifndef _KERN_TTY_H
#define _KERN_TTY_H 1

// Get size_t.
#include <stddef.h>
#include <ds/ring.h>

#define TTY_MUX  0
#define TTY_VGA  1
#define TTY_COM0 2
#define NTTYS    3

#define TTY_BUFSIZ 512

typedef ring_buffer(char, TTY_BUFSIZ) tty_queue_t;

struct tty_struct
{
	int (*putc)(int);
	int (*getc)(int *);
	tty_queue_t read_q;
	struct task *read_wait;
};

struct tty_struct ttys[NTTYS];

void tty_intr(int num);
size_t tty_read(int num, char *buf, size_t n);
void tty_write(int num, const char *buf, size_t n);
void tty_register(int num, int (*putc)(int), int (*getc)(int *));

#endif

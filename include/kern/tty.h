#ifndef _KERN_TTY_H
#define _KERN_TTY_H 1

// Get size_t.
#include <stddef.h>
#include <bits/termios.h>
#include <ds/ring.h>

#define TTY_COM0 0
#define TTY_VGA  1
#define NTTYS    2
#ifdef _TTY_SERIAL
#define TTY_STD TTY_COM0
#else
#define TTY_STD TTY_VGA
#endif

#define TTY_BUFSIZ 512

typedef sring_t(char, TTY_BUFSIZ) tty_queue_t;

struct tty_struct
{
	int (*putc)(int);
	int (*getc)(int *);
	tty_queue_t read_q;
	struct task *read_wait;
	struct termios tc;
};

struct tty_struct ttys[NTTYS];

void tty_intr(int num);
size_t tty_read(int num, char *buf, size_t n);
void tty_write(int num, const char *buf, size_t n);
void tty_register(int num, int (*putc)(int), int (*getc)(int *));
int tty_ioctl(int num, int req, long arg);

#endif

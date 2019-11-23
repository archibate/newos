#include <kern/tty.h>
#include <kern/sched.h>
#include <bits/ioctl.h>
#include <string.h>
#include <errno.h>

struct tty_struct ttys[NTTYS];

static void
tty_putc(struct tty_struct *tty, int c)
{
	tty->putc(c);
}

static int
tty_getc(struct tty_struct *tty)
{
	int c;
	while (sring_empty(&tty->read_q))
		block_on(&tty->read_wait);
	// tty_intr filled one, we pop it
	sring_get(&tty->read_q, c);
	return c;
}

void
tty_intr(int num)
{
	int c;
	struct tty_struct *tty = &ttys[num];
	while (tty->getc(&c)) {
		if (sring_full(&tty->read_q)) {
			if (tty->tc.c_iflag & IMAXBEL)
				tty_putc(tty, tty->tc.c_cc[VBELL]);
		} else {
			if ((tty->tc.c_lflag & ECHO)
				|| ((tty->tc.c_lflag & ECHONL)
					&& c == tty->tc.c_cc[VEOL]))
				tty_putc(tty, c);
			sring_put(&tty->read_q, c);
			if (!(tty->tc.c_lflag & ICANON)
				|| c == tty->tc.c_cc[VEOL]
				|| c == tty->tc.c_cc[VEOF])
				wake_up(&tty->read_wait);
		}
	}
}

void
tty_write(int num, const char *buf, size_t n)
{
	size_t i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		int c = buf[i++];
		tty_putc(tty, c);
	}
}

size_t
tty_read(int num, char *buf, size_t n)
{
	size_t i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		int c = tty_getc(tty);
		buf[i++] = c;
		if (!(tty->tc.c_lflag & ICANON)
			|| c == tty->tc.c_cc[VEOL]
			|| c == tty->tc.c_cc[VEOF])
			break;
	}
	return i;
}

void
tty_register(int num, int (*putc)(int), int (*getc)(int *))
{
	struct tty_struct *tty = &ttys[num];
	tty->putc = putc;
	tty->getc = getc;
	tty->tc = (struct termios) DEFAULT_TERMIOS;
}

int
tty_ioctl(int num, int req, long arg)
{
	struct tty_struct *tty = &ttys[num];
	switch (req) {
	case I_TC_GETATTR:
		memcpy((void *)arg, &tty->tc, sizeof(struct termios));
		return 0;
	case I_TC_SETATTR:
		memcpy(&tty->tc, (const void *)arg, sizeof(struct termios));
		return 0;
	}
	errno = EINVAL;
	return -1;
}

#include <kern/tty.h>
#include <kern/sched.h>
#include <bits/ioctl.h>
#include <bits/notify.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

struct tty_struct ttys[NTTYS];

static void
tty_putc(struct tty_struct *tty, int c)
{
	tty->putc(c);
}

static int
tty_getc(struct tty_struct *tty, int *c)
{
	while (sring_empty(&tty->read_q)) {
		if (sleep_on(&tty->read_wait))
			return -1;
	}
	// tty_intr filled one, we pop it
	sring_get(&tty->read_q, *c);
	return 0;
}
static void
tty_put_printable_c(struct tty_struct *tty, int c)
{
	if (c == tty->tc.c_cc[VEOL])
		goto ok_to_print;
	if (!isprint(c)) {
		tty_putc(tty, '^');
		c |= '@';
		c &= 0x7f;
		if (c == 0x7f)
			c = '?';
	}
ok_to_print:
	tty_putc(tty, c);
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
				tty_put_printable_c(tty, c);
			sring_put(&tty->read_q, c);
			if ((tty->tc.c_lflag & ISIG) &&
					c == tty->tc.c_cc[VINTR])
				goto wake;
			if (!(tty->tc.c_lflag & ICANON)
					|| c == tty->tc.c_cc[VEOL]
					|| c == tty->tc.c_cc[VEOF])
				goto wake;

			continue;
		wake:
			wake_up(&tty->read_wait);
			if (tty->notify) {
				do_kill(tty->notify, SIGPOLL);
				tty->notify = NULL;
			}
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
		int c;
		if (tty_getc(tty, &c) == -1)
			break;
		if ((tty->tc.c_lflag & ICANON) && c == tty->tc.c_cc[VEOF])
			break;
		if ((tty->tc.c_lflag & ISIG) && c == tty->tc.c_cc[VINTR]) {
			sys_raise(SIGINT);
			break;
		}
		buf[i++] = c;
		if (!(tty->tc.c_lflag & ICANON) || c == tty->tc.c_cc[VEOL])
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

int
tty_ionotify(int num, int flags)
{
	if (flags & ~ION_READ) {
		errno = EINVAL;
		return -1;
	}
	if (!(flags & ION_READ))
		return 0;
	struct tty_struct *tty = &ttys[num];
	if (tty->notify) {
		errno = EAGAIN;
		return -1;
	}
	tty->notify = current;
	return 0;
}

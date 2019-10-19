#include <kern/tty.h>

static struct tty_struct ttys[NTTYS];

static void
tty_putc(struct tty_struct *tty, int c)
{
	tty->putc(c);
}

static int
tty_getc(struct tty_struct *tty)
{
	int c;
	while (RING_EMPTY(&tty->read_q)) {
		// TODO: use sleep_on here (DEP sched, cli)
	}
	// tty_intr filled one, we pop it
	RING_GET(&tty->read_q, c);
	return c;
}

void
tty_intr(int num)
{
	int c;
	struct tty_struct *tty = &ttys[num];
	while (tty->getc(&c)) {
		if (RING_FULL(&tty->read_q)) {
			tty_putc(tty, '\a');
		} else {
			tty_putc(tty, c);
			RING_PUT(&tty->read_q, c);
		}
	}
}

void
tty_write(int num, const char *buf, size_t n)
{
	int c, i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		c = buf[i++];
		tty_putc(tty, c);
	}
}

size_t
tty_read(int num, char *buf, size_t n)
{
	int c, i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		int c = tty_getc(tty);
		buf[i++] = c;
		if (c == '\n')
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
}

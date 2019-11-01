#include <kern/tty.h>
#include <kern/sched.h>

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
	while (ring_empty(&tty->read_q))
		block_on(&tty->read_wait);
	// tty_intr filled one, we pop it
	ring_get(&tty->read_q, c);
	return c;
}

void
tty_intr(int num)
{
	int c;
	struct tty_struct *tty = &ttys[num];
	while (tty->getc(&c)) {
		if (ring_full(&tty->read_q)) {
			tty_putc(tty, '\a');
		} else {
			tty_putc(tty, c);
			ring_put(&tty->read_q, c);
			if (c == '\n')
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

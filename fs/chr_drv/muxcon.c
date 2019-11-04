#include <kern/tty.h>
#include <kern/kernel.h>

int muxcon_putc(int c)
{
	char buf[1];
	buf[0] = c;
	tty_write(TTY_COM0, buf, 1);
	tty_write(TTY_VGA, buf, 1);
	return 0;
}

void muxcon_intr(int tty)
{
	ttys[TTY_MUX].getc = ttys[tty].getc;
	tty_intr(TTY_MUX);
}

void
muxcon_init(void)
{
	tty_register(TTY_MUX, muxcon_putc, NULL);
}

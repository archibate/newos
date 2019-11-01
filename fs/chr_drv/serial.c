#include <kern/asm/serial.h>
#include <kern/irq.h>
#include <kern/tty.h>
#include <sys/io.h>

static int serial_exists;

static void
delay(void)
{
	inb(0x84);
	inb(0x84);
	inb(0x84);
	inb(0x84);
}

static int
rs_putc(int c)
{
	for (int i = 0; !(inb(COM0+COM_LSR) & COM_LSR_TXRDY) && i < 12800; i++)
		delay();
	outb(COM0+COM_RX, c);
	return 0;
}

static int
rs_getc(int *pc)
{
	if (!(inb(COM0+COM_LSR) & COM_LSR_DATA))
		return 0;
	int c = inb(COM0+COM_RX);
	// Enter is interpreted as '\r' in serial
	// let's convert it.
	if (c == '\r')
		c = '\n';
	*pc = c;
	return 1;
}

static void
rs_intr(void)
{
	if (serial_exists)
		tty_intr(TTY_COM0);
}

void
rs_init(void)
{
	// Turn off the FIFO
	outb(COM0+COM_FCR, 0);

	// Set speed; requires DLAB latch
	outb(COM0+COM_LCR, COM_LCR_DLAB);
	outb(COM0+COM_DLL, (unsigned char) (115200 / 9600));
	outb(COM0+COM_DLM, 0);

	// 8 data bits, 1 stop bit, parity off; turn off DLAB latch
	outb(COM0+COM_LCR, COM_LCR_WLEN8 & ~COM_LCR_DLAB);

	// No modem controls
	outb(COM0+COM_MCR, 0);
	// Enable rcv interrupts
	outb(COM0+COM_IER, COM_IER_RDI);

	// Clear any preexisting overrun indications and interrupts
	// Serial port doesn't exist if COM_LSR returns 0xff
	serial_exists = inb(COM0+COM_LSR) != 0xff;
	(void) inb(COM0+COM_IIR);
	(void) inb(COM0+COM_RX);

	tty_register(TTY_COM0, rs_putc, rs_getc);
	install_irq(IRQ_COM0, rs_intr);
	irq_enable(IRQ_COM0, 1);
}

#include <sys/io.h>
#include <kern/kernel.h>
#include <kern/irq.h>
#include <kern/tty.h>

static int serial_exists;

static void
delay(void)
{
	inb(0x84);
	inb(0x84);
	inb(0x84);
	inb(0x84);
}

#define COM0	0x3f8

#define COM_RX		0	// In:	Receive buffer (DLAB=0)
#define COM_TX		0	// Out: Transmit buffer (DLAB=0)
#define COM_DLL		0	// Out: Divisor Latch Low (DLAB=1)
#define COM_DLM		1	// Out: Divisor Latch High (DLAB=1)
#define COM_IER		1	// Out: Interrupt Enable Register
#define   COM_IER_RDI	0x01	//   Enable receiver data interrupt
#define COM_IIR		2	// In:	Interrupt ID Register
#define COM_FCR		2	// Out: FIFO Control Register
#define COM_LCR		3	// Out: Line Control Register
#define	  COM_LCR_DLAB	0x80	//   Divisor latch access bit
#define	  COM_LCR_WLEN8	0x03	//   Wordlength: 8 bits
#define COM_MCR		4	// Out: Modem Control Register
#define	  COM_MCR_RTS	0x02	// RTS complement
#define	  COM_MCR_DTR	0x01	// DTR complement
#define	  COM_MCR_OUT2	0x08	// Out2 complement
#define COM_LSR		5	// In:	Line Status Register
#define   COM_LSR_DATA	0x01	//   Data available
#define   COM_LSR_TXRDY	0x20	//   Transmit buffer avail
#define   COM_LSR_TSRE	0x40	//   Transmitter off

static int
rs_putc(int c)
{
	for (int i = 0; !(inb(COM0+COM_LSR) & COM_LSR_TXRDY) && i < 12800; i++)
		delay();
	outb(COM0+COM_RX, c);
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
	// Serial port doesn't exist if COM_LSR returns 0xFF
	serial_exists = inb(COM0+COM_LSR) != 0xff;
	(void) inb(COM0+COM_IIR);
	(void) inb(COM0+COM_RX);

	tty_register(TTY_COM0, rs_putc, rs_getc);
	install_irq(IRQ_COM0, rs_intr);
	irq_enable(IRQ_COM0, 1);
}

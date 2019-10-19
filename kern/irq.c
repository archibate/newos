#include <kern/irq.h>
#include <kern/kernel.h>
#include <sys/io.h>

#define PIC0  0x20
#define PIC1  0xa0

void
irq_init(void)
{
	outb(PIC0+1, 0xfb);
	outb(PIC1+1, 0xff);

	outb(PIC0+0, 0x11);
	outb(PIC0+1, 0x20);
	outb(PIC0+1, 1<<2);
	outb(PIC0+1, 0x03);

	outb(PIC1+0, 0x11);
	outb(PIC1+1, 0x20 + 8);
	outb(PIC1+1,    2);
	outb(PIC1+1, 0x01);

	outb(PIC0+1, 0xfb); // PIC0: all masked except slave PIC1
	outb(PIC1+1, 0xff); // PIC1: all masked
}

void
irq_enable(int irq, int enable)
{
	unsigned char port = PIC0+1;
	if (irq >= 8) {
		port = PIC1+1;
		irq -= 8;
	}
	unsigned char mask = inb(port);
	if (enable)
		mask &= 0xff ^ (1 << irq);
	else
		mask |= 1 << irq;
	outb(port, mask);
}

void
irq_done(int irq)
{
	outb(PIC0, 0x20);
	if (irq >= 8)
		outb(PIC1, 0x20); // 0xa0?
}

static void (*irq_proc[NIRQS])(void);

void
install_irq(int irq, void (*proc)(void))
{
	irq_proc[irq] = proc;
}

void
do_irq(int irq)
{
	if (irq_proc[irq])
		irq_proc[irq]();
	else
		printk("Ignored IRQ #%d", irq);
	irq_done(irq);
}

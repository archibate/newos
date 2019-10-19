#include <kern/clock.h>
#include <kern/irq.h>
#include <sys/io.h>

clock_t clock_tick;

void do_clock(void)
{
	clock_tick++;
}

#define PIT_CNTL  0x43
#define PIT_CTR0  0x40

void clock_init(void)
{
	int ctr = 1193180 / CLOCKS_PER_SEC;

	outb(PIT_CNTL, 0x34);
	outb(PIT_CTR0, ctr & 0xff);
	outb(PIT_CTR0, ctr >> 8);

	install_irq(IRQ_PIT, do_clock);
	irq_enable(IRQ_PIT, 1);
}

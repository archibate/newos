#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/irq.h>

extern void do_syscall(reg_t *regs);
extern void do_fault(reg_t *regs);

void
do_trap(reg_t eax, ...)
{
	reg_t *regs = &eax;
	unsigned nr = regs[INTRNO];
	if (nr >= 0x20 && nr <= 0x20 + NIRQS) {
		do_irq(nr - 0x20);
	} else if (nr == 0x80) {
		do_syscall(regs);
	} else {
		do_fault(regs);
	}
}

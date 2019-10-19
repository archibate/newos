#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/irq.h>
#include <stddef.h>

static const char *exception_string[] = {
	"Divide error",
	"Debug",
	"Non-Maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection",
	"Page Fault",
	"(unknown trap)",
	"x87 FPU Floating-Point Error",
	"Alignment Check",
	"Machine-Check",
	"SIMD Floating-Point Exception",
};

void
do_trap(unsigned long eax, ...)
{
	unsigned long *reg = &eax;
	unsigned long nr = reg[INTRNO];
	if (nr < array_sizeof(exception_string)) {
		panic("INT %#x: %s", nr, exception_string[nr]);
	} else if (nr >= 0x20 && nr <= 0x20 + NIRQS) {
		do_irq(nr - 0x20);
	} else {
		panic("Unknown INT %#x", nr);
	}
}

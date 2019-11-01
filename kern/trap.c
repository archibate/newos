#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/irq.h>
#include <stddef.h>

static const char *const exception_string[] = {
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

static void
dump_context(unsigned long *reg)
{
	unsigned long cr0, cr2, cr3, cr4;
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	asm volatile ("mov %%cr2, %0" : "=r" (cr2));
	asm volatile ("mov %%cr3, %0" : "=r" (cr3));
	asm volatile ("mov %%cr4, %0" : "=r" (cr4));
	printk("CR0=%p  CR2=%p  CR3=%p  CR4=%p",
		cr0, cr2, cr3, cr4);
	printk("EAX=%p  EBX=%p  ECX=%p  EDX=%p",
		reg[EAX], reg[EBX], reg[ECX], reg[EDX]);
	printk("ESP=%p  EBP=%p  ESI=%p  EDI=%p",
		reg[ESP], reg[EBP], reg[ESI], reg[EDI]);
	printk("CS=%#04X  SS=%#04X  DS=%#04X  ES=%#04X  FS=%#04X  GS=%#04X",
		reg[CS], reg[SS], reg[DS], reg[ES], reg[FS], reg[GS]);
	printk("EIP=%p  EFLAGS=%#08X  INTRNO=%d  ERRCODE=%d",
		reg[EIP], reg[EFLAGS], reg[INTRNO], reg[ERRCODE]);
}

extern int do_page_fault(unsigned long *reg);

void
do_trap(unsigned long eax, ...)
{
	unsigned long *reg = &eax;
	unsigned long nr = reg[INTRNO];
	if (nr < array_sizeof(exception_string)) {
		if (nr == 0x0e && do_page_fault(reg))
			return;
		dump_context(reg);
		panic("INT %#x: %s", nr, exception_string[nr]);
	} else if (nr >= 0x20 && nr <= 0x20 + NIRQS) {
		do_irq(nr - 0x20);
	} else {
		panic("Unknown INT %#x", nr);
	}
}

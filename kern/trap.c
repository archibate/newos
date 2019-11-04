#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/irq.h>
#include <sys/intrin.h>
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
dump_context(const reg_t *regs)
{
	printk("CR0=%p  CR2=%p  CR3=%p  CR4=%p",
		scr0(), scr2(), scr3(), scr4());
	printk("EAX=%p  EBX=%p  ECX=%p  EDX=%p",
		regs[EAX], regs[EBX], regs[ECX], regs[EDX]);
	printk("ESP=%p  EBP=%p  ESI=%p  EDI=%p",
		regs[ESP], regs[EBP], regs[ESI], regs[EDI]);
	printk("CS=%#04X  SS=%#04X  DS=%#04X  ES=%#04X  FS=%#04X  GS=%#04X",
		regs[CS], regs[SS], regs[DS], regs[ES], regs[FS], regs[GS]);
	printk("EIP=%p  EFLAGS=%#08X  INTRNO=%d  ERRCODE=%#X",
		regs[EIP], regs[EFLAGS], regs[INTRNO], regs[ERRCODE]);
}

extern int do_page_fault(reg_t *regs);
extern void do_syscall(reg_t *regs);

void
do_trap(reg_t eax, ...)
{
	reg_t *regs = &eax;
	unsigned nr = regs[INTRNO];
	if (nr < array_sizeof(exception_string)) {
		if (nr == 0x0e && do_page_fault(regs))
			return;
		dump_context(regs);
		panic("INT %#x: %s", nr, exception_string[nr]);
	} else if (nr >= 0x20 && nr <= 0x20 + NIRQS) {
		do_irq(nr - 0x20);
	} else if (nr == 0x80) {
		do_syscall(regs);
	} else {
		panic("Unknown INT %#x", nr);
	}
}

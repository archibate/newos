#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <sys/intrin.h>
#include <stddef.h>

extern int do_page_fault(reg_t *regs);

static const char *const exception_string[] = {
	"Divide error",				// 0x00: SIGFPE
	"Debug",				// 0x01: SIGTRAP
	"Non-Maskable Interrupt",		// 0x02
	"Breakpoint",				// 0x03: SIGTRAP
	"Overflow",				// 0x04: SIGFPE
	"BOUND Range Exceeded",			// 0x05: SIGSEGV
	"Invalid Opcode",			// 0x06: SIGILL
	"Device Not Available",			// 0x07
	"Double Fault",				// 0x08: SIGSEGV
	"Coprocessor Segment Overrun",		// 0x09: SIGSEGV
	"Invalid TSS",				// 0x0a: SIGSEGV
	"Segment Not Present",			// 0x0b: SIGSEGV
	"Stack Fault",				// 0x0c: SIGSEGV
	"General Protection",			// 0x0d: SIGSEGV
	"Page Fault",				// 0x0e: SIGSEGV
	"(unknown trap)",			// 0x0f
	"x87 FPU Floating-Point Error",		// 0x10: SIGFPE
	"Alignment Check",			// 0x11: SIGSEGV
	"Machine-Check",			// 0x12: SIGSEGV
	"SIMD Floating-Point Exception",	// 0x13: SIGFPE
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

void do_fault(reg_t *regs)
{
	unsigned nr = regs[INTRNO];
	int sig = -1;
	if (nr == 0x0e) {
		switch (do_page_fault(regs)) {
		case 0:
			sig = SIGSEGV;
			goto dump;
		case 1:
			return;
		default:
			goto panic;
		}
	}
	if ((regs[CS] & 0x3) != 0x3)
		goto panic;
	switch (nr) {
	case 0x01: case 0x03:
		sig = SIGTRAP;
		goto raise;
	case 0x06:
		sig = SIGILL;
		break;
	case 0x00: case 0x10: case 0x13:
		sig = SIGFPE;
		break;
	case 0x05: case 0x08: case 0x09:
	case 0x0a: case 0x0b: case 0x0d:
	case 0x11: case 0x12:
		sig = SIGSEGV;
		break;
	default:
		goto panic;
	}
dump:
	printk(">>> CORE DUMP BEGIN <<<");
	printk("Process %d: %s", current->pid, exception_string[nr]);
	dump_context(regs);
	printk(">>>> CORE DUMP END <<<<");
raise:
	if (sig != -1)
		sys_raise_a(sig, -1);
	return;
panic:
	dump_context(regs);
	if (nr < array_sizeof(exception_string))
		panic("INT %#02x: %s", nr, exception_string[nr]);
	else
		panic("Unknown INT %#02x", nr);
}

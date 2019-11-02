#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/tty.h>

void do_syscall(unsigned long *reg)
{
	switch (reg[EAX]) {
	case 1:
		tty_write(TTY_COM0, (const void *)reg[ECX], reg[EDX]);
		tty_write(TTY_VGA,  (const void *)reg[ECX], reg[EDX]);
		reg[EAX] = reg[EDX];
		break;
	default:
		printk("WARNING: Unknown system call %d", reg[EAX]);
		reg[EAX] = -1;
		break;
	}
}

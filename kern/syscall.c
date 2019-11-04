#include <sys/reg.h>
#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/tty.h>
#include <errno.h>

int sys_debugi(int x)
{
	printk("%d sys_debugi %d", current->pid, x);
	return x;
}

void do_syscall(reg_t *regs)
{
#define _DEFINE_KERNEL_SYSCALL_SWITCH 1
#include <kern/syscall.h>
}

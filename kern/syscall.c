#include <sys/reg.h>
#include <kern/kernel.h>
#include <kern/tty.h>
#include <errno.h>

void do_syscall(reg_t *regs)
{
#define _DEFINE_KERNEL_SYSCALL_SWITCH 1
#include <kern/syscall.h>
}

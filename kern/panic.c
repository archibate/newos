#include <kern/kernel.h>
#include <sys/intrin.h>

__attribute__((noreturn)) void
do_panic(void)
{
	printk("panic occurred, system halted");
	clihlt();
}

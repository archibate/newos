#include <kern/kernel.h>

__attribute__((noreturn)) void
do_panic(void)
{
	printk("panic occurred, system halted");
	for (;;) asm volatile ("cli;hlt");
}

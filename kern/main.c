#include <kern/kernel.h>
#include <kern/sched.h>
#include <malloc.h>

#define INIT(x) do { \
	void x##_init(void); \
	x##_init(); \
} while (0)

int initial_thread(void *arg)
{
	printk("th1 got [%s]", arg);
	return 0;
}

void
main(void)
{
	*(int*)0xb8000 = 0x0f6b0f4f;
	INIT(gdt);
	INIT(idt);
	INIT(irq);
	INIT(rs);
	INIT(vga);
	INIT(memtest);
	INIT(bootmm);
	INIT(pmm);
	INIT(clock);
	INIT(rtc);
	INIT(sched);
	printk("Kernel Started");

	/** do some tests begin **/
	free(malloc(100)); // FIXME: 莫名其妙地把我接下来分配的tcb清零
	struct task *p = kernel_thread(initial_thread, "Hello, th1!");
	p->priority = 1;
	/** do some tests end **/

	asm volatile ("sti");
	for (;;)
		asm volatile ("hlt");
}

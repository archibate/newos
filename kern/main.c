#include <kern/kernel.h>
#include <kern/tty.h>

#define INIT(x) do { \
	void x##_init(void); \
	x##_init(); \
} while (0)

void
main(void)
{
	*(int*)0xb8000 = 0x0f6b0f4f;
	INIT(gdt);
	INIT(idt);
	INIT(irq);
	INIT(rs);
	INIT(vga);
	INIT(clock);
	printk("Kernel Started");
	asm volatile ("sti");

	char buf[10];
	buf[tty_read(TTY_COM0, buf, sizeof(buf))] = 0;
	printk("[%s]", buf);
	for (;;)
		asm volatile ("hlt");
}

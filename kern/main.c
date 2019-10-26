#include <kern/kernel.h>
#include <kern/sched.h>
#include <kern/tty.h>
#include <kern/fs.h>
#include <malloc.h>

#define INIT(x) do { \
	void x##_init(void); \
	x##_init(); \
} while (0)

int initial_thread(void *arg)
{
	printk("th got [%s]", arg);
	char c = 0;
	tty_read(TTY_COM0, &c, 1);
	printk("th [%s] got [%c]", arg, c);
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
	printk("Kernel Started");
	INIT(memtest);
	INIT(bootmm);
	INIT(pmm);
	INIT(clock);
	INIT(rtc);
	INIT(sched);

	/** do some tests begin **/
	free(malloc(100));
	kernel_thread(initial_thread, "Hello, th1!");
	kernel_thread(initial_thread, "Hello, th2!");
	struct inode *ip = iget(NEFS_ROOT_INO);
	char buf[233];
	buf[0] = 'b';
	buf[1] = '!';
	iwrite(ip, 0x3600, buf, 2);
	iread(ip, 0x3600, buf, sizeof(buf));
	printk("[%.7s]", buf);
	printk("ip->i_size = %d", ip->i_size);
	iwrite(ip, ip->i_size, buf, 1028);
	printk("ip->i_size = %d", ip->i_size);
	iwrite(ip, ip->i_size, buf, 2);
	printk("ip->i_size = %d", ip->i_size);
	buf[3] = '!';
	iread(ip, ip->i_size - 2, buf, 9);
	printk("[%.7s]", buf);
	/** do some tests end **/

	asm volatile ("sti");
	for (;;)
		asm volatile ("hlt");
}

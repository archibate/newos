#include <kern/kernel.h>
#include <kern/sched.h>
#include <kern/tty.h>
#include <kern/fs.h>
#include <string.h>
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
	INIT(fs);

	/** do some tests begin **/
	free(malloc(100));
	kernel_thread(initial_thread, "Hello, th1!");
	kernel_thread(initial_thread, "Hello, th2!");
	struct inode *ip;
	struct dir_entry de;
	char buf[233];
	printk("ls of /");
	ip = namei("/");
	for (int i = 0; -1 != dir_read_entry(ip, &de, i); i++) {
		if (de.d_ino != 0) {
			printk("%6d %.*s", de.d_ino, NEFS_NAME_MAX, de.d_name);
		}
	}
	iput(ip);
	ip = creati("hello.txt", 1);
	strcpy(buf, "Hello, World!\n");
	iwrite(ip, 0, buf, sizeof(buf));
	iput(ip);
	printk("ls of .");
	ip = namei(".");
	for (int i = 0; -1 != dir_read_entry(ip, &de, i); i++) {
		if (de.d_ino != 0) {
			printk("%6d %.*s", de.d_ino, NEFS_NAME_MAX, de.d_name);
		}
	}
	iput(ip);
	ip = namei("aa.txt");
	if (!ip) panic("aa.txt not found");
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
	iput(ip);
	printk("contents of /usr/src/snake.c");
	ip = namei("../usr/src/snake.c");
	if (!ip) panic("../usr/src/snake.c not found");
	size_t s, pos = 0;
	while ((s = iread(ip, pos, buf, sizeof(buf)))) {
		tty_write(TTY_COM0, buf, s);
		pos += s;
	}
	iput(ip);
	ip = namei("hello.txt");
	if (!ip) panic("hello.txt not found");
	pos = 0;
	while ((s = iread(ip, pos, buf, sizeof(buf)))) {
		tty_write(TTY_COM0, buf, s);
		pos += s;
	}
	iput(ip);
	/** do some tests end **/

	asm volatile ("sti");
	for (;;)
		asm volatile ("hlt");
}

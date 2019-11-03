#include <kern/kernel.h>
#include <kern/sched.h>
#include <kern/tty.h>
#include <kern/fs.h>
#include <kern/mm.h>
#include <kern/exec.h>
#include <sys/intrin.h>
#include <string.h>
#include <malloc.h>

#define INIT(x) do { \
	void x##_init(void); \
	x##_init(); \
} while (0)

int initial_thread(const char *path)
{
	struct inode *ip = namei(path);
	if (!ip) panic("cannot open %s", path);
	if (do_execve(ip) == -1) panic("cannot exec %s", path);
	iput(ip);
	move_to_user();
}

void
main(void)
{
	*(int*)0xb8000 = 0x0f6b0f4f;
	INIT(gdt);
	INIT(idt);
	INIT(tss);
	INIT(irq);
	INIT(rs);
	INIT(vga);
	INIT(muxcon);
	printk("Kernel Started");
	INIT(memtest);
	INIT(bootmm);
	INIT(pmm);
	INIT(clock);
	INIT(rtc);
	INIT(sched);
	INIT(fs);

	kernel_thread(initial_thread, "/bin/xiaomin");

	sti();
	for (;;)
		hlt();
}

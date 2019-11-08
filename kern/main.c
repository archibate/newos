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
	char *argv[] = {(char *)path, "-s", NULL};
	char *envp[] = {"PATH=/bin", "HOME=/root", NULL};
	current->filp[0] = fs_open("/dev/tty/com0", O_RDONLY, 0); // muxin has BUG
	current->filp[1] = fs_open("/dev/tty/mux", O_WRONLY, 0);
	current->filp[2] = current->filp[1] ? fs_dup(current->filp[1]) : NULL;
	sys_execve(path, argv, envp);
	panic("cannot exec %s", path);
}

#if 0 // {{{
struct task *queue;

int test_thread_1(void *unused)
{
	printk("1 blocking on...");
	block_on(&queue);
	printk("1 unblocked!");
	return 0;
}

int test_thread_2(void *unused)
{
	printk("2 waking up...");
	wake_up(&queue);
	return 0;
}
#endif // }}}

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

	//kernel_thread(test_thread_1, NULL);
	//kernel_thread(test_thread_2, NULL);
	kernel_thread(initial_thread, "/bin/init");

	sti();
	for (;;)
		hlt();
}

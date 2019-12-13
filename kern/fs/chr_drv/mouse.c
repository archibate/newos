#ifdef _VIDEO
#include <kern/fs.h>
#include <kern/sched.h>
#include <bits/ioctl.h>
#include <bits/notify.h>
#include <kern/kernel.h>
#include <kern/irq.h>
#include <ds/ring.h>
#include <sys/io.h>
#include <errno.h>

static sring_t(char, 1024) mouse_q;
struct task *mouse_wait, *mouse_notify;
long mouse_notify_arg;

int ionotify_dev_mouse(int flags, long arg)
{
	if (flags & ~ION_READ) {
		errno = EINVAL;
		return -1;
	}
	if (!(flags & ION_READ))
		return 0;
	if (mouse_notify) {
		errno = EAGAIN;
		return -1;
	}
	mouse_notify_arg = arg;
	mouse_notify = current;
	return 0;
}

static void mouse_intr(void)
{
	static int phase = 1, button, x, y;
	unsigned char data = inb(0x60);

	switch (phase) {
	case 0:
		if (data == 0xfa)
			phase = 1;
		return;
	case 1:
		if ((data & 0xc8) == 0x08) {
			phase = 2;
			button = data;
		}
		return;
	case 2:
		phase = 3;
		x = data & 0xff;
		return;
	case 3:
		phase = 1;
		y = data & 0xff;
		if (button & 0x10)
			x |= 0xffffff00;
		if (button & 0x20)
			y |= 0xffffff00;
		y = -y;
		if (sring_left(&mouse_q) <= sring_capacity(&mouse_q) - 4) {
			sring_put(&mouse_q, 27);
			sring_put(&mouse_q, button & 0xff);
			sring_put(&mouse_q, x & 0xff);
			sring_put(&mouse_q, y & 0xff);
			wake_up(&mouse_wait);
			if (mouse_notify) {
				do_kill(mouse_notify, SIGPOLL, mouse_notify_arg);
				mouse_notify = NULL;
			}
		}
		return;
	}
}

size_t rw_dev_mouse(int rw, size_t pos, void *buf, size_t size)
{
	if (rw != READ) {
		errno = -EPERM;
		return 0;
	}
	int saw = 0;
	for (size_t i = 0; i < size; i++) {
		while (sring_empty(&mouse_q)) {
			if (saw) return i;
			saw = 1;
			//printk("sleep_on");
			if (sleep_on(&mouse_wait)) {
				//printk("sleep_bad");
				return i;
			}
			//printk("sleep_off");
		}
		sring_get(&mouse_q, i[(char *)buf]);
	}
	return size;
}

int ioctl_dev_mouse(int req, long arg)
{
	switch (req) {
	case I_CLBUF:
		sring_init(&mouse_q);
		return 0;
	}
	errno = EINVAL;
	return -1;
}

void mouse_init(void)
{
	while (inb(0x64) & 0x02);
	outb(0x64, 0x60);
	while (inb(0x64) & 0x02);
	outb(0x60, 0x47);
	while (inb(0x64) & 0x02);
	outb(0x64, 0xd4);
	while (inb(0x64) & 0x02);
	outb(0x60, 0xf4);
	install_irq(IRQ_MOUSE, mouse_intr);
	irq_enable(IRQ_MOUSE, 1);
}
#endif

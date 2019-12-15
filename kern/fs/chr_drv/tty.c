#include <kern/fs.h>
#include <kern/tty.h>
#include <kern/sched.h>
#include <bits/ioctl.h>
#include <bits/notify.h>
#include <kern/kernel.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct tty_struct ttys[NTTYS];

static void
tty_putc(struct tty_struct *tty, int c)
{
	tty->putc(c, tty - ttys);
}

static int
tty_getc(struct tty_struct *tty, int *c)
{
	while (sring_empty(&tty->read_q)) {
		if (sleep_on(&tty->read_wait))
			return -1;
	}
	// tty_intr filled one, we pop it
	sring_get(&tty->read_q, *c);
	return 0;
}

static void
tty_put_printable_c(struct tty_struct *tty, int c)
{
	if (c == tty->tc.c_cc[VEOL])
		goto ok_to_print;
	if (!isprint(c)) {
		tty_putc(tty, '^');
		c |= '@';
		c &= 0x7f;
		if (c == 0x7f)
			c = '?';
	}
ok_to_print:
	tty_putc(tty, c);
}

void
tty_intr(int num)
{
	int c;
	struct tty_struct *tty = &ttys[num];
	while (tty->getc(&c, num)) {
		if (sring_full(&tty->read_q)) {
			if (tty->tc.c_iflag & IMAXBEL)
				tty_putc(tty, tty->tc.c_cc[VBELL]);
		} else {
			if ((tty->tc.c_lflag & ECHO)
				|| ((tty->tc.c_lflag & ECHONL)
					&& c == tty->tc.c_cc[VEOL]))
				tty_put_printable_c(tty, c);
			sring_put(&tty->read_q, c);
			if ((tty->tc.c_lflag & ISIG) &&
					c == tty->tc.c_cc[VINTR])
				goto wake;
			if (!(tty->tc.c_lflag & ICANON)
					|| c == tty->tc.c_cc[VEOL]
					|| c == tty->tc.c_cc[VEOF])
				goto wake;

			continue;
		wake:
			wake_up(&tty->read_wait);
			if (tty->notify) {
				do_kill(tty->notify, SIGPOLL, tty->notify_arg);
				tty->notify = NULL;
			}
		}
	}
}

void
tty_write(int num, const char *buf, size_t n)
{
	size_t i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		int c = buf[i++];
		tty_putc(tty, c);
	}
}

size_t
tty_read(int num, char *buf, size_t n)
{
	int c;
	size_t i = 0;
	struct tty_struct *tty = &ttys[num];
	while (i < n) {
		if (tty_getc(tty, &c) == -1)
			break;
		if ((tty->tc.c_lflag & ICANON) && c == tty->tc.c_cc[VEOF])
			break;
		if ((tty->tc.c_lflag & ISIG) && c == tty->tc.c_cc[VINTR]) {
			sys_raise_a(SIGINT, -2);
			break;
		}
		buf[i++] = c;
		if (!(tty->tc.c_lflag & ICANON) || c == tty->tc.c_cc[VEOL])
			break;
	}
	return i;
}


void
tty_register(int num, int (*putc)(), int (*getc)())
{
	struct tty_struct *tty = &ttys[num];
	tty->putc = putc;
	tty->getc = getc;
	tty->tc = (struct termios) DEFAULT_TERMIOS;
	tty->valid = 1;
}

int
tty_ioctl(int num, int req, long arg)
{
	struct tty_struct *tty = &ttys[num];
	switch (req) {
	case I_TC_GETATTR:
		memcpy((void *)arg, &tty->tc, sizeof(struct termios));
		return 0;
	case I_TC_SETATTR:
		memcpy(&tty->tc, (const void *)arg, sizeof(struct termios));
		return 0;
	}
	errno = EINVAL;
	return -1;
}

int
tty_ionotify(int num, int flags, long arg)
{
	struct tty_struct *tty = &ttys[num];
	if (flags & ~ION_READ) {
		errno = EINVAL;
		return -1;
	}
	if (!(flags & ION_READ))
		return 0;
	if (tty->notify) {
		errno = EAGAIN;
		return -1;
	}
	tty->notify = current;
	tty->notify_arg = arg;
	return 0;
}

int
pty_putc(int c, int num)
{
	struct tty_struct *tty = &ttys[num];
	char data[1];
	data[0] = c;
	//printk("ptyputc: [%c]", c);
	if (pipe_write(tty->pty.write_p, data, 1) != 1)
		return 0;
	return 1;
}

int
pty_getc(int *c, int num)
{
	struct tty_struct *tty = &ttys[num];
	char data[1];
	if (pipe_read(tty->pty.read_p, data, 1) != 1) {
		//printk("??? %d", num);
		return 0;
	}
	*c = data[0];
	//printk("ptygetc: [%c]", *c);
	return 1;
}

static int
alloc_pty(void)
{
	int i;
	for (i = TTY_PTS0; i < NTTYS; i++) {
		if (!ttys[i].valid)
			goto found;
	}
	errno = ENOMEM;
	return -1;
found:
	tty_register(i, pty_putc, pty_getc);
	struct tty_struct *tty = &ttys[i];
	tty->pty.read_p = make_pipe();
	tty->pty.write_p = make_pipe();
	tty->pty.read_p->i_p_ptyp1 = i + 1;
	return i;
}

int
ptmx_open(struct file *f)
{
	int num = alloc_pty();
	if (num == -1)
		return -1;
	struct inode *pts;
	char path[233];
	sprintf(path, "/dev/pts/%d", num - TTY_PTS0);
	pts = creati(path, 1, S_IFCHR | 0666, DEV_TTY0 + num);
	if (!pts) {
		errno = EIO;
		return -1;
	}
	iput(pts);
	struct tty_struct *tty = &ttys[num];
	f->f_type = FT_IMUX;
	f->f_ip = idup(tty->pty.write_p);
	f->f_wip = idup(tty->pty.read_p);
	return 0;
}

// notice that tty_intr is very like ionotify callback?

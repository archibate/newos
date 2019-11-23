#include <kern/fs.h>
#include <kern/tty.h>
#include <string.h>
#include <errno.h>

static size_t rw_dev_null(int rw, off_t pos, void *buf, size_t size)
{
	if (rw == WRITE)
		return size;
	return 0;
}

static size_t rw_dev_zero(int rw, off_t pos, void *buf, size_t size)
{
	if (rw == WRITE)
		return size;
	memset(buf, 0, size);
	return size;
}

size_t chr_drv_rw(int rw, int nr, off_t pos, void *buf, size_t size)
{
	if (nr >= DEV_TTY0) {
		nr -= DEV_TTY0;
		if (rw == WRITE)
			tty_write(nr, buf, size);
		else
			size = tty_read(nr, buf, size);
	} else switch (nr) {
	case DEV_NULL: return rw_dev_null(rw, pos, buf, size);
	case DEV_ZERO: return rw_dev_zero(rw, pos, buf, size);
	}
	return size;
}

int chr_drv_ioctl(int nr, int req, long arg)
{
	if (nr >= DEV_TTY0) {
		nr -= DEV_TTY0;
		return tty_ioctl(nr, req, arg);
	}
	errno = ENOTTY;
	return -1;
}

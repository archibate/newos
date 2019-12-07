#include <kern/fs.h>
#include <kern/tty.h>
#include <string.h>
#include <errno.h>

extern int ioctl_dev_fb0(int req, long arg);
extern int ioctl_dev_mouse(int req, long arg);
extern int ionotify_dev_mouse(int flags);
extern size_t rw_dev_fb0(int rw, size_t pos, void *buf, size_t size);
size_t rw_dev_mouse(int rw, size_t pos, void *buf, size_t size);

static size_t rw_dev_null(int rw, size_t pos, void *buf, size_t size)
{
	if (rw == WRITE)
		return size;
	return 0;
}

static size_t rw_dev_zero(int rw, size_t pos, void *buf, size_t size)
{
	if (rw == WRITE)
		return size;
	memset(buf, 0, size);
	return size;
}

size_t chr_drv_rw(int rw, int nr, size_t pos, void *buf, size_t size)
{
	if (nr >= DEV_TTY0) {
		nr -= DEV_TTY0;
		if (rw == WRITE)
			tty_write(nr, buf, size);
		else
			size = tty_read(nr, buf, size);
		return size;
	}
	switch (nr) {
	case DEV_NULL: return rw_dev_null(rw, pos, buf, size);
	case DEV_ZERO: return rw_dev_zero(rw, pos, buf, size);
#ifdef _VIDEO
	case DEV_FB0: return rw_dev_fb0(rw, pos, buf, size);
	case DEV_MOUSE: return rw_dev_mouse(rw, pos, buf, size);
#endif
	}
	errno = ENODEV;
	return 0;
}

ssize_t chr_drv_seek(int nr, size_t pos)
{
	switch (nr) {
	case DEV_FB0: return pos;
	}
	errno = ESPIPE;
	return -1;
}

int chr_drv_ioctl(int nr, int req, long arg)
{
	if (nr >= DEV_TTY0) {
		nr -= DEV_TTY0;
		return tty_ioctl(nr, req, arg);
	}
	switch (nr) {
#ifdef _VIDEO
	case DEV_FB0: return ioctl_dev_fb0(req, arg);
	case DEV_MOUSE: return ioctl_dev_mouse(req, arg);
#endif
	}
	errno = ENODEV;
	return -1;
}

int chr_drv_ionotify(int nr, int flags)
{
	if (nr >= DEV_TTY0) {
		nr -= DEV_TTY0;
		return tty_ionotify(nr, flags);
	}
	switch (nr) {
#ifdef _VIDEO
	case DEV_MOUSE: return ionotify_dev_mouse(flags);
#endif
	}
	errno = ENODEV;
	return -1;
}

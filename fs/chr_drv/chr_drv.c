#include <kern/fs.h>
#include <kern/tty.h>

size_t chr_drv_rw(int rw, int nr, off_t pos, void *buf, size_t size)
{
	if (rw == WRITE)
		tty_write(nr, buf, size);
	else
		size = tty_read(nr, buf, size);
	return size;
}

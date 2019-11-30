#include <kern/fs.h>
#include <bits/ioctl.h>
#include <kern/kernel.h>
#include <string.h>
#include <errno.h>

static char *vram;
static size_t vram_size;
static size_t vram_nx, vram_ny;

#ifdef _VIDEO
void video_init(void)
{
	size_t i;
	vram = *(char **)0x7ff4;
	vram_nx = *(short *)0x7ff8;
	vram_ny = *(short *)0x7ffa;
	vram_size = vram_nx * vram_ny;
	if (!(vram && !((long)vram & 0x7fff))) {
		panic("cannot get video infomation from boot stage");
	}
	printk("vram at %p (%dx%d)", vram, vram_nx, vram_ny);
}

size_t rw_dev_fb0(int rw, size_t pos, void *buf, size_t size)
{
	if (pos > vram_size) {
		errno = EINVAL;
		return 0;
	}
	if (pos + size > vram_size)
		size = vram_size - pos;

	if (rw == WRITE) { 
		memcpy(vram + pos, buf, size);
	} else {
		memcpy(buf, vram + pos, size);
	}

	return size;
}

int ioctl_dev_fb0(int req, long arg)
{
	switch (req) {
	case I_FB_GET_NX:
		return vram_nx;
	case I_FB_GET_NY:
		return vram_ny;
	}
	errno = EINVAL;
	return -1;
}
#endif

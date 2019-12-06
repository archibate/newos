#ifdef _VIDEO
#include <kern/fs.h>
#include <bits/ioctl.h>
#include <kern/kernel.h>
#include <sys/io.h>
#include <string.h>
#include <errno.h>

static char *vram;
static size_t vram_size;
static size_t vram_nx, vram_ny;

static void palette_init(void)
{
	outb(0x3c8, 0);
	for (int i = 0; i < 256; i++) {
		outb(0x3c9, (i & 3) << 4);
		outb(0x3c9, ((i >> 2) & 7) << 3);
		outb(0x3c9, ((i >> 5) & 7) << 3);
	}
}

void video_init(void)
{
	vram = *(char **)0x7ff4;
	if (!(vram && !((long)vram & 0x7fff)))
		panic("cannot get video info from boot stage");
	vram_nx = *(short *)0x7ff8;
	vram_ny = *(short *)0x7ffa;
	vram_size = vram_nx * vram_ny;
	palette_init();
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

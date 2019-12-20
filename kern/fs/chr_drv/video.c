#ifdef _VIDEO
#include <kern/fs.h>
#include <kern/mm.h>
#include <bits/ioctl.h>
#include <kern/kernel.h>
#include <sys/io.h>
#include <string.h>
#include <errno.h>

static char *vram;
static size_t vram_size;
static size_t vram_nx, vram_ny;
static int vram_bpp;

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
	vram_bpp = *(short *)0x7ffc;
	vram_size = vram_nx * vram_ny * vram_bpp / 8;
	palette_init();
	printk("vram at %p (%dx%d)", vram, vram_nx, vram_ny);
	if (vram > (char *)KERNEL_PMAP) {
		pte_t pte, *vram_ptes;
		pde_t pde, *pd;
		viraddr_t va;
		vram_ptes = boot_alloc(PAGEUP(vram_size));
		for (va = KERNEL_VRAM,
				pte = (pte_t)vram | PG_W | PG_P;
				va < KERNEL_VRAM + PAGEUP(vram_size);
				va += PGSIZE, pte += PGSIZE)
			vram_ptes[(va - KERNEL_VRAM) >> 12] = pte;
		pd = mmu_get_pgdir();
		for (va = KERNEL_VRAM,
				pde = (pde_t)vram_ptes | PG_W | PG_P;
				va < KERNEL_VRAM + (vram_size + PGSIZE * PGSIZE - 1)
				/ PGSIZE / PGSIZE * PGSIZE * PGSIZE;
				va += PGSIZE * PGSIZE, pde += PGSIZE)
			pd[va >> 22] = pde;
		vram = (char *)KERNEL_VRAM;
		mmu_set_pgdir(pd);
		printk("mapped vram at %p", vram);
	}
	for (size_t i = 0; i < vram_size; i++) {
		vram[i] = i & 0xff;
	}
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
	case I_FB_GET_BPP:
		return vram_bpp;
	}
	errno = EINVAL;
	return -1;
}
#endif

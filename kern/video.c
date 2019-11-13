#include <kern/kernel.h>

void video_init(void)
{
	char *vram = *(char **)0x7ff8;
	if (vram && !((long)vram & 0x7fff)) {
		printk("vram at %p", vram);
		for (int i = 0; i < 320 * 200; i++) {
			vram[i] = i % 16;
		}
	}
}

#include <kern/tss.h>
#include <kern/gdt.h>
#include <kern/idt.h>
#include <sys/intrin.h>

struct tss tss0;

void tss_init(void)
{
	extern char boot_stack_top[0];
	tss0.ts_ss0 = SEG_KDATA;
	tss0.ts_esp0 = (unsigned long)boot_stack_top;
	tss0.ts_iomb = sizeof(tss0) - 1;
	tss0.ts_iomap[0] = 0xff;

	gdt[SEG_TSS0 >> 3] = SEGD16(STS_T32A, (unsigned long)&tss0, sizeof(tss0), 0);
	gdt[SEG_TSS0 >> 3].sd_s = 0;

	ltr(SEG_TSS0);
}

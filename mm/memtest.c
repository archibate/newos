#include <kern/mm.h>
#include <kern/asm/cmos.h>
#include <kern/kernel.h>
#include <sys/intrin.h>
#include <stddef.h>

size_t total_pages;
physaddr_t base_mem_end;

void memtest_init(void)
{
	unsigned *mem, memkb, cr0, a;
	size_t base = nvram_read(NVRAM_BASEMEM);
	size_t ext  = nvram_read(NVRAM_EXTMEM);
	total_pages = (base + ext) >> 2;

	//extern int ebss;
	//memkb = ((size_t)&ebss + 1024*1024 - 1) >> 20;
	memkb = total_pages >> 8;
	mem = (unsigned *)(memkb << 20);

	// store a copy of CR0
	cr0 = scr0();

	// invalidate the cache
	// write-back and invalidate the cache
	wbinvd();

	// plug cr0 with just PE/CD/NW
	// cache disable(486+), no-writeback(486+), 32bit mode(386+)
	lcr0(cr0 | 0x60000001);

	while (memkb < 4096) {

		a = *mem;
		*mem = 0x55aa55aa;
		// the empty asm calls tell gcc not to rely on what's in its registers
		// as saved variables (this avoids GCC optimisations)
		asm ("" ::: "memory");
		if (*mem != 0x55aa55aa)
			break;

		*mem = 0xaa55aa55;
		asm ("" ::: "memory");
		if(*mem != 0xaa55aa55)
			break;

		asm ("" ::: "memory");
		*mem = a;

		memkb++;
		mem += 1024*1024;
	}

	lcr0(cr0);

	total_pages = memkb << 8;
	mem = (unsigned *)0x413; // BIOS memory
	base_mem_end = (*mem & 0xFFFF) << 6;

	printk("physical memory %u MB", memkb);
}

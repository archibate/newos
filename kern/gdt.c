#include <kern/gdt.h>

struct segdesc gdt[NGDTS] =
{
	SEGD_NULL,
	SEGD(STA_X | STA_R, 0x0, 0xffffffff, 0), // Kernel Code
	SEGD(STA_W,         0x0, 0xffffffff, 0), // Kernel Data
	SEGD(STA_X | STA_R, 0x0, 0xffffffff, 3), // User Code
	SEGD(STA_W,         0x0, 0xffffffff, 3), // User Data
	SEGD_NULL,                               // Reserved for TSS
};

void
gdt_init(void)
{
	static struct gdtr {
		unsigned short limit;
		unsigned long base;
	} __attribute__((packed)) gdtr = {
		.limit = sizeof(gdt) - 1,
		.base = (unsigned long) gdt,
	};
	asm volatile ("lgdt (%0)" :: "r" (&gdtr));
	asm volatile (
			"mov %0, %%ss\n"
			"mov %0, %%ds\n"
			"mov %0, %%es\n"
			"mov %0, %%fs\n"
			"mov %0, %%gs\n"
			:: "a" (0x10)
		     );
	asm volatile (
			"push %0\n"
			"push $1f\n"
			"retf\n"
			"1:\n"
			:: "a" (0x08)
		     );
}

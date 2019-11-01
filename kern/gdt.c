#include <kern/gdt.h>
#include <sys/intrin.h>

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
	lgdt(&gdtr);
	set_ss(0x10);
	set_ds(0x10);
	set_es(0x10);
	set_fs(0x10);
	set_gs(0x10);
	set_cs(0x08);
}

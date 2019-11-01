#include <kern/idt.h>
#include <sys/intrin.h>

struct gatedesc idt[NIDTS];

void
idt_init(void)
{
	extern unsigned long idt_entry_table[NIDTS];
	for (int i = 0; i < NIDTS; i++)
		SETGATE(idt[i], 0, 0x08, idt_entry_table[i], i == 0x80 ? 3 : 0);
	static struct idtr {
		unsigned short limit;
		unsigned long base;
	} __attribute__((packed)) idtr = {
		.limit = sizeof(idt) - 1,
		.base = (unsigned long) idt,
	};
	lidt(&idtr);
}

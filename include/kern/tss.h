#ifndef _KERN_TSS_H
#define _KERN_TSS_H 1

/************ BEGIN *************/
/**** Again, Thanks for JOS *****/
/* Source: https://github.com/archibate/jos/blob/master/lab4/partC/inc/mmu.h */

// Task state segment format (as described by the Pentium architecture book)
struct tss {
	unsigned int ts_link;	// Old ts selector
	unsigned long ts_esp0;	// Stack pointers and segment selectors
	unsigned short ts_ss0;	//   after an increase in privilege level
	unsigned short ts_padding1;
	unsigned long ts_esp1;
	unsigned short ts_ss1;
	unsigned short ts_padding2;
	unsigned long ts_esp2;
	unsigned short ts_ss2;
	unsigned short ts_padding3;
	unsigned long ts_cr3;	// Page directory base
	unsigned long ts_eip;	// Saved state from last task switch
	unsigned int ts_eflags;
	unsigned int ts_eax;	// More saved state (registers)
	unsigned int ts_ecx;
	unsigned int ts_edx;
	unsigned int ts_ebx;
	unsigned long ts_esp;
	unsigned long ts_ebp;
	unsigned int ts_esi;
	unsigned int ts_edi;
	unsigned short ts_es;		// Even more saved state (segment selectors)
	unsigned short ts_padding4;
	unsigned short ts_cs;
	unsigned short ts_padding5;
	unsigned short ts_ss;
	unsigned short ts_padding6;
	unsigned short ts_ds;
	unsigned short ts_padding7;
	unsigned short ts_fs;
	unsigned short ts_padding8;
	unsigned short ts_gs;
	unsigned short ts_padding9;
	unsigned short ts_ldt;
	unsigned short ts_padding10;
	unsigned short ts_t;		// Trap on task switch
	unsigned short ts_iomb;	// I/O map base address
	unsigned char ts_iomap[1]; // I/O map; Fake array
} __attribute__((packed));

extern struct tss tss0;

#endif

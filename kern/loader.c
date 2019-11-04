#include <kern/exec.h>
#include <kern/kernel.h>
#include <kern/gdt.h>
#include <kip/elf.h>
#include <string.h>

static int mm_load_exec_elf(struct mm_struct *mm, reg_t *regs, struct inode *ip)
{
	struct elf32_ehdr e;
	struct elf32_phdr ph;
	iread(ip, 0, &e, sizeof(e));
	if (e.e_magic != ELF_MAGIC)
		return -1;
	if (e.e_bits != ELF_32)
		return -1;
	if (e.e_endian != ELF_LE)
		return -1;
	if (e.e_machine != EM_386)
		return -1;
	if (e.e_type != ET_EXEC)
		return -1;

	viraddr_t ebss = 0;

	for (size_t i = 0; i < e.e_phnum; i++) {
		iread(ip, e.e_phoff + i * e.e_phentsz, &ph, sizeof(ph));
		if (ph.p_type != PT_LOAD)
			continue;
		size_t filesz = PAGEUP(ph.p_filesz);
		size_t memsz = PAGEUP(ph.p_memsz);
		// PF_* and PROT_* are exactly same, so we can do so:
		mm_new_area(mm, ph.p_vaddr, filesz,
				ph.p_flags & 7, MAP_PRIVATE, ip, ph.p_offset);
		if (PGOFFS(ph.p_vaddr)) {
			printk("WARNING: unaligned ph#%d/%p not loaded", i, ph.p_vaddr);
			continue;
		}
		if (memsz > filesz)
			mm_new_area(mm, ph.p_vaddr + filesz, memsz - filesz,
					ph.p_flags & 7, MAP_PRIVATE, NULL, 0);
		if (ebss < ph.p_vaddr + memsz)
			ebss = ph.p_vaddr + memsz;
	}

	mm_new_area(mm, ebss, USER_STACK_SIZE,
			PROT_READ | PROT_WRITE, MAP_PRIVATE, NULL, 0);

	regs[EBP] = ebss;
	regs[ESP] = ebss + USER_STACK_SIZE - 16;
	regs[EIP] = e.e_entry;
	regs[EFLAGS] = 0x202;
	regs[SS] = SEG_UDATA;
	regs[DS] = SEG_UDATA;
	regs[ES] = SEG_UDATA;
	regs[FS] = SEG_UDATA;
	regs[GS] = SEG_UDATA;
	regs[CS] = SEG_UCODE;

	return 0;
}

int mm_load_exec(struct mm_struct *mm, reg_t *regs, struct inode *ip)
{
	union {
		char buf[4];
		int magic;
	} u;
	memset(&u, 0, sizeof(u));
	iread(ip, 0, &u.buf, sizeof(u));
	if (u.magic == ELF_MAGIC)
		return mm_load_exec_elf(mm, regs, ip);
	else if (u.buf[0] == '#' && u.buf[1] == '!')
		panic("sh!bang not supported yet");
	return -1;
}

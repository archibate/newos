#include <kern/exec.h>
#include <kern/kernel.h>
#include <kern/gdt.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <elf.h>

#define irdexec(...) rw_inode(READ, __VA_ARGS__)
#define bad_load() sys_exit(-1)

static int mm_load_dyn_elf(struct mm_struct *mm, struct inode *ip,
		viraddr_t *addr, viraddr_t *disp);

static void load_dynamics(struct mm_struct *mm, viraddr_t addr,
		struct inode *ip, off_t offset, void *dyna)
{
	int ret;
	char *name;
	off_t dlt_pos;
	viraddr_t disp;
	struct elf32_dlhdr dl;
	struct inode *dlip;
	for (dlt_pos = offset;
		irdexec(ip, dlt_pos, &dl, sizeof(dl)) == sizeof(dl) && dl.d_namelen;
		dlt_pos += sizeof(dl) + dl.d_namelen);
	dlt_pos += sizeof(dl);
	viraddr_t *dlt = dyna + dlt_pos - offset;
	for (;	irdexec(ip, offset, &dl, sizeof(dl)) == sizeof(dl) && dl.d_namelen;
		offset += sizeof(dl) + dl.d_namelen) {
		if ((unsigned)dl.d_namelen > PATH_MAX)
			bad_load();
		char name[dl.d_namelen + 1];
		irdexec(ip, offset + sizeof(dl), name, dl.d_namelen);
		name[dl.d_namelen] = 0;
		//printk("load_dynamic %p %s (%d symbols)", addr, name, dl.d_dltents);
		dlip = namei(name);
		if (dlip) {
			ret = mm_load_dyn_elf(mm, dlip, &addr, &disp);
			iput(dlip);
		} else {
			printk("ld.so: %s: %s", name, strerror(errno));
			ret = -1;
		}
		if (ret == -1) {
			printk("ld.so: cannot load shared library %s", name);
			bad_load();
		}
		if (disp) {
			for (size_t i = 0; i < dl.d_dltents; i++) {
				dlt[i] += disp;
			}
		}
		dlt += dl.d_dltents;
	}
	return;
}

static int mm_load_dyn_elf(struct mm_struct *mm, struct inode *ip,
		viraddr_t *addr, viraddr_t *disp)
{
	struct elf32_ehdr e;
	struct elf32_phdr ph;
	irdexec(ip, 0, &e, sizeof(e));
	if (e.e_magic != ELF_MAGIC)
		return -1;
	if (e.e_bits != ELF_32)
		return -1;
	if (e.e_endian != ELF_LE)
		return -1;
	if (e.e_machine != EM_386)
		return -1;
	if (e.e_type != ET_DYN)
		return -1;

	for (size_t i = 0; i < e.e_phnum; i++) {
		irdexec(ip, e.e_phoff + i * e.e_phentsz, &ph, sizeof(ph));
		if (!i) {
			*disp = *addr - ph.p_vaddr;
			if (PGOFFS(*disp)) {
				printk("ld.so: unaligned starting address %p", ph.p_vaddr);
				return -1;
			}
		}
		if (ph.p_type != PT_LOAD)
			continue;
		size_t filesz = PAGEUP(ph.p_filesz);
		size_t memsz = PAGEUP(ph.p_memsz);
		//printk("mm_new_area %p %p", ph.p_vaddr + *disp, filesz);
		// PF_* and PROT_* are exactly same, so we can do so:
		if (!mm_new_area(mm, ph.p_vaddr + *disp, filesz,
				ph.p_flags & 7, MAP_FIXED_NOREPLACE, ip, ph.p_offset))
			return -1;
		if (PGOFFS(ph.p_vaddr)) {
			printk("ld.so: unaligned ph#%d/%p not loaded", i, ph.p_vaddr);
			continue;
		}
		if (memsz > filesz)
		//printk("mm_new_area %p %p zero", ph.p_vaddr + *disp + filesz, memsz, filesz),
			mm_new_area(mm, ph.p_vaddr + *disp + filesz, memsz - filesz,
					ph.p_flags & 7, MAP_FIXED_NOREPLACE, NULL, 0);
		*addr = ph.p_vaddr + *disp + memsz;
	}

	return 0;
}

static int mm_load_exec_elf(struct mm_struct *mm, reg_t *regs, struct inode *ip)
{
	struct elf32_ehdr e;
	struct elf32_phdr ph;
	irdexec(ip, 0, &e, sizeof(e));
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

	if (current->mm)
		free_mm(current->mm);
	switch_to_mm(current->mm = mm);

	viraddr_t ebss = 0;

	for (size_t i = 0; i < e.e_phnum; i++) {
		irdexec(ip, e.e_phoff + i * e.e_phentsz, &ph, sizeof(ph));
		if (ph.p_type != PT_LOAD)
			continue;
		size_t filesz = PAGEUP(ph.p_filesz);
		size_t memsz = PAGEUP(ph.p_memsz);
		// PF_* and PROT_* are exactly same, so we can do so:
		if (!mm_new_area(mm, ph.p_vaddr, filesz,
				ph.p_flags & 7, MAP_FIXED_NOREPLACE, ip, ph.p_offset)) {
			bad_load();
		}
		if (PGOFFS(ph.p_vaddr)) {
			printk("ld.so: unaligned ph#%d/%p not loaded", i, ph.p_vaddr);
			continue;
		}
		if (memsz > filesz)
			mm_new_area(mm, ph.p_vaddr + filesz, memsz - filesz,
					ph.p_flags & 7, MAP_FIXED_NOREPLACE, NULL, 0);
		if (ebss < ph.p_vaddr + memsz)
			ebss = ph.p_vaddr + memsz;
	}
	for (size_t i = 0; i < e.e_phnum; i++) {
		irdexec(ip, e.e_phoff + i * e.e_phentsz, &ph, sizeof(ph));
		if (ph.p_type == PT_DYNAMIC) {
			load_dynamics(mm, 0xbffff000, ip, ph.p_offset, (void *)ph.p_vaddr);
			continue;
		}
	}

	mm_new_area(mm, ebss, USER_STACK_SIZE,
			PROT_READ | PROT_WRITE, MAP_FIXED_NOREPLACE, NULL, 0);

	mm->ebss = ebss;
	mm->ebrk = ebss;
	mm->stop = ebss + USER_STACK_SIZE;
	regs[EBP] = mm->ebss;
	regs[ESP] = mm->stop - 16;
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
	irdexec(ip, 0, &u.buf, sizeof(u));
	if (u.magic == ELF_MAGIC)
		return mm_load_exec_elf(mm, regs, ip);
	else if (u.buf[0] == '#' && u.buf[1] == '!')
		printk("sh!bang not supported yet");
	return -1;
}

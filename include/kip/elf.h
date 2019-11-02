#ifndef _KIP_ELF_H
#define _KIP_ELF_H 1

#include <stdint.h>

// For e_magic, ASCII of "\x7fELF":
#define ELF_MAGIC 0x464c457f

// For e_type:
#define ET_NONE	0
#define ET_REL	1
#define ET_EXEC	2
#define ET_DYN	3
#define ET_CORE	4

// For e_machine:
#define EM_NONE		0
#define EM_386		3
#define EM_MIPS		8
#define EM_ARM		0x28
#define EM_X86_64	0x3e
#define EM_AARCH64	0xb7

// For e_bits:
#define ELF_32	1
#define ELF_64	2

// For e_endian:
#define ELF_LE	1
#define ELF_BE	2

struct elf32_ehdr
{
	uint32_t e_magic;
	uint8_t e_bits;
	uint8_t e_endian;
	uint8_t e_hdrver;
	uint8_t e_os_abi;
	uint8_t e_unused[8];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_hdrsize;
	uint16_t e_phentsz;
	uint16_t e_phnum;
	uint16_t e_shentsz;
	uint16_t e_shnum;
	uint16_t e_snameidx;
};

// For p_type:
#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4

// For p_flags:
#define PF_X	1
#define PF_W	2
#define PF_R	4

struct elf32_phdr
{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

#endif

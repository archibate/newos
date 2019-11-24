#ifndef _ELF_H
#define _ELF_H 1

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

#if 0 // {{{
/* Legal values for d_tag (dynamic entry type).  */

#define DT_NULL		0		/* Marks end of dynamic section */
#define DT_NEEDED	1		/* Name of needed library */
#define DT_PLTRELSZ	2		/* Size in bytes of PLT relocs */
#define DT_PLTGOT	3		/* Processor defined value */
#define DT_HASH		4		/* Address of symbol hash table */
#define DT_STRTAB	5		/* Address of string table */
#define DT_SYMTAB	6		/* Address of symbol table */
#define DT_RELA		7		/* Address of Rela relocs */
#define DT_RELASZ	8		/* Total size of Rela relocs */
#define DT_RELAENT	9		/* Size of one Rela reloc */
#define DT_STRSZ	10		/* Size of string table */
#define DT_SYMENT	11		/* Size of one symbol table entry */
#define DT_INIT		12		/* Address of init function */
#define DT_FINI		13		/* Address of termination function */
#define DT_SONAME	14		/* Name of shared object */
#define DT_RPATH	15		/* Library search path (deprecated) */
#define DT_SYMBOLIC	16		/* Start symbol search here */
#define DT_REL		17		/* Address of Rel relocs */
#define DT_RELSZ	18		/* Total size of Rel relocs */
#define DT_RELENT	19		/* Size of one Rel reloc */
#define DT_PLTREL	20		/* Type of reloc in PLT */
#define DT_DEBUG	21		/* For debugging; unspecified */
#define DT_TEXTREL	22		/* Reloc might modify .text */
#define DT_JMPREL	23		/* Address of PLT relocs */
#define	DT_BIND_NOW	24		/* Process relocations of object */
#define	DT_INIT_ARRAY	25		/* Array with addresses of init fct */
#define	DT_FINI_ARRAY	26		/* Array with addresses of fini fct */
#define	DT_INIT_ARRAYSZ	27		/* Size in bytes of DT_INIT_ARRAY */
#define	DT_FINI_ARRAYSZ	28		/* Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH	29		/* Library search path */
#define DT_FLAGS	30		/* Flags for the object being loaded */
#define DT_ENCODING	32		/* Start of encoded range */
#define DT_PREINIT_ARRAY 32		/* Array with addresses of preinit fct*/
#define DT_PREINIT_ARRAYSZ 33		/* size in bytes of DT_PREINIT_ARRAY */
#define DT_SYMTAB_SHNDX	34		/* Address of SYMTAB_SHNDX section */
#define	DT_NUM		35		/* Number used */

struct elf32_dyn
{
	int32_t d_tag;
	union {
		uint32_t d_val;
		uint32_t d_ptr;
	};
};
#endif // }}}

struct elf32_dlhdr
{
	uint32_t d_dltents;
	uint32_t d_namelen;
};

#endif

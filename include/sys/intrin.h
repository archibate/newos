#ifndef _SYS_INTRIN_H
#define _SYS_INTRIN_H 1

static inline unsigned long
scr0(void)
{
	unsigned long cr0;
	asm volatile ("mov %%cr0, %0" : "=r" (cr0) :: "cc", "memory");
	return cr0;
}

static inline void
lcr0(unsigned long cr0)
{
	asm volatile ("mov %0, %%cr0" :: "r" (cr0) : "cc", "memory");
}

static inline unsigned long
scr2(void)
{
	unsigned long cr2;
	asm volatile ("mov %%cr2, %0" : "=r" (cr2) :: "cc", "memory");
	return cr2;
}

static inline void
lcr2(unsigned long cr2)
{
	asm volatile ("mov %0, %%cr2" :: "r" (cr2) : "cc", "memory");
}

static inline unsigned long
scr3(void)
{
	unsigned long cr3;
	asm volatile ("mov %%cr3, %0" : "=r" (cr3) :: "cc", "memory");
	return cr3;
}

static inline void
lcr3(unsigned long cr3)
{
	asm volatile ("mov %0, %%cr3" :: "r" (cr3) : "cc", "memory");
}

static inline unsigned long
scr4(void)
{
	unsigned long cr4;
	asm volatile ("mov %%cr4, %0" : "=r" (cr4) :: "cc", "memory");
	return cr4;
}

static inline void
lcr4(unsigned long cr4)
{
	asm volatile ("mov %0, %%cr4" :: "r" (cr4) : "cc", "memory");
}

static inline unsigned short
get_cs(void)
{
	unsigned short cs;
	asm volatile ("mov %%cs, %0" : "=r" (cs) :: "cc", "memory");
	return cs;
}

static inline void
set_cs(unsigned long cs)
{
	asm volatile ("push %0\npush $1f\nretf\n1:" :: "r" (cs) : "cc", "memory");
}

static inline unsigned short
get_ss(void)
{
	unsigned short ss;
	asm volatile ("mov %%ss, %0" : "=r" (ss) :: "cc", "memory");
	return ss;
}

static inline void
set_ss(unsigned long ss)
{
	asm volatile ("mov %0, %%ss" :: "r" (ss) : "cc", "memory");
}

static inline unsigned short
get_ds(void)
{
	unsigned short ds;
	asm volatile ("mov %%ds, %0" : "=r" (ds) :: "cc", "memory");
	return ds;
}

static inline void
set_ds(unsigned long ds)
{
	asm volatile ("mov %0, %%ds" :: "r" (ds) : "cc", "memory");
}

static inline unsigned short
get_es(void)
{
	unsigned short es;
	asm volatile ("mov %%es, %0" : "=r" (es) :: "cc", "memory");
	return es;
}

static inline void
set_es(unsigned long es)
{
	asm volatile ("mov %0, %%es" :: "r" (es) : "cc", "memory");
}

static inline unsigned short
get_fs(void)
{
	unsigned short fs;
	asm volatile ("mov %%fs, %0" : "=r" (fs) :: "cc", "memory");
	return fs;
}

static inline void
set_fs(unsigned long fs)
{
	asm volatile ("mov %0, %%fs" :: "r" (fs) : "cc", "memory");
}

static inline unsigned short
get_gs(void)
{
	unsigned short gs;
	asm volatile ("mov %%gs, %0" : "=r" (gs) :: "cc", "memory");
	return gs;
}

static inline void
set_gs(unsigned long gs)
{
	asm volatile ("mov %0, %%gs" :: "r" (gs) : "cc", "memory");
}

static inline unsigned int
seflags(void)
{
	unsigned int eflags;
	asm volatile ("pushfl\npop %0" : "=r" (eflags) :: "cc", "memory");
	return eflags;
}

static inline void
leflags(unsigned int eflags)
{
	asm volatile ("push %0\npopfl" :: "r" (eflags) : "cc", "memory");
}

static inline void
sgdt(void *gdtr)
{
	asm volatile ("sgdt (%0)" :: "r" (gdtr) : "cc", "memory");
}

static inline void
lgdt(const void *gdtr)
{
	asm volatile ("lgdt (%0)" :: "r" (gdtr) : "cc", "memory");
}

static inline void
sidt(void *idtr)
{
	asm volatile ("sidt (%0)" :: "r" (idtr) : "cc", "memory");
}

static inline void
lidt(const void *idtr)
{
	asm volatile ("lidt (%0)" :: "r" (idtr) : "cc", "memory");
}

static inline unsigned short
sldt(void)
{
	unsigned short ldtr;
	asm volatile ("sldt %%ax" : "=a" (ldtr) :: "cc", "memory");
	return ldtr;
}

static inline void
ltr(unsigned long tr)
{
	asm volatile ("ltr %%ax" :: "a" (tr) : "cc", "memory");
}

static inline unsigned short
str(void)
{
	unsigned short tr;
	asm volatile ("str %%ax" : "=a" (tr) :: "cc", "memory");
	return tr;
}

static inline void
lldt(unsigned long ldtr)
{
	asm volatile ("lldt %%ax" :: "a" (ldtr) : "cc", "memory");
}

static inline void
invlpg(void *p)
{
	asm volatile ("invlpg (%0)" :: "r" (p) : "cc", "memory");
}

static inline void
wbinvd(void)
{
	asm volatile ("wbinvd" ::: "cc", "memory");
}

static inline void
sti(void)
{
	asm volatile ("sti" ::: "cc", "memory");
}

static inline void
cli(void)
{
	asm volatile ("cli" ::: "cc", "memory");
}

static inline void
hlt(void)
{
	asm volatile ("hlt" ::: "cc", "memory");
}

static inline void
stihlt(void)
{
	asm volatile ("sti;hlt" ::: "cc", "memory");
}

__attribute__((noreturn)) static inline void
clihlt(void)
{
	while (1)
		asm volatile ("cli;hlt" ::: "cc", "memory");
}

static inline void
hltcli(void)
{
	asm volatile ("hlt;cli" ::: "cc", "memory");
}

static inline void
stihltcli(void)
{
	asm volatile ("sti;hlt;cli" ::: "cc", "memory");
}

static inline void
nop(void)
{
	asm volatile ("nop" ::: "cc", "memory");
}

static inline void
ud(void)
{
	asm volatile ("ud" ::: "cc", "memory");
}

static inline void
ud2(void)
{
	asm volatile ("ud2" ::: "cc", "memory");
}

static inline void
farjmp(unsigned int seg, unsigned long off)
{
	asm volatile ("push %0\npush %1\nretf" :: "r" (seg), "r" (off) : "cc", "memory");
}

#endif

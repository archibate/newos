#ifndef _SYS_IO_H
#define _SYS_IO_H 1

static inline unsigned char
inb(unsigned short port)
{
	unsigned char data;
	asm volatile ("in %%dx, %%al\n" : "=a" (data) : "d" (port));
	return data;
}

static inline void
outb(unsigned short port, unsigned char data)
{
	asm volatile ("out %%al, %%dx\n" :: "a" (data), "d" (port));
}

static inline unsigned short
inw(unsigned short port)
{
	unsigned short data;
	asm volatile ("in %%dx, %%ax\n" : "=a" (data) : "d" (port));
	return data;
}

static inline void
outw(unsigned short port, unsigned short data)
{
	asm volatile ("out %%ax, %%dx\n" :: "a" (data), "d" (port));
}

static inline unsigned int
inl(unsigned short port)
{
	unsigned int data;
	asm volatile ("in %%dx, %%eax\n" : "=a" (data) : "d" (port));
	return data;
}

static inline void
outl(unsigned short port, unsigned int data)
{
	asm volatile ("out %%eax, %%dx\n" :: "a" (data), "d" (port));
}

static inline void
insb(unsigned short port, void *buf, unsigned long len)
{
	asm volatile ("rep;insb" :: "D" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static inline void
outsb(unsigned short port, const void *buf, unsigned long len)
{
	asm volatile ("rep;outsb" :: "S" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static inline void
insw(unsigned short port, void *buf, unsigned long len)
{
	asm volatile ("rep;insw" :: "D" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static inline void
outsw(unsigned short port, const void *buf, unsigned long len)
{
	asm volatile ("rep;outsw" :: "S" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static inline void
insl(unsigned short port, void *buf, unsigned long len)
{
	asm volatile ("rep;insl" :: "D" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static inline void
outsl(unsigned short port, const void *buf, unsigned long len)
{
	asm volatile ("rep;outsl" :: "S" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}
#endif

#ifndef _SYS_IO_H
#define _SYS_IO_H 1

static unsigned char
inb(unsigned short port)
{
	unsigned char data;
	asm volatile ("in %%dx, %%al\n" : "=a" (data) : "d" (port));
	return data;
}

static void
outb(unsigned short port, unsigned char data)
{
	asm volatile ("out %%al, %%dx\n" :: "a" (data), "d" (port));
}

static unsigned short
inw(unsigned short port)
{
	unsigned short data;
	asm volatile ("in %%dx, %%ax\n" : "=a" (data) : "d" (port));
	return data;
}

static void
outw(unsigned short port, unsigned short data)
{
	asm volatile ("out %%ax, %%dx\n" :: "a" (data), "d" (port));
}

static unsigned int
inl(unsigned short port)
{
	unsigned int data;
	asm volatile ("in %%dx, %%eax\n" : "=a" (data) : "d" (port));
	return data;
}

static void
outl(unsigned short port, unsigned int data)
{
	asm volatile ("out %%eax, %%dx\n" :: "a" (data), "d" (port));
}

static void
outsl(unsigned short port, const void *buf, unsigned long len)
{
	asm volatile ("rep;outsl" :: "S" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static void
insl(unsigned short port, void *buf, unsigned long len)
{
	asm volatile ("rep;insl" :: "D" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static void
outsb(unsigned short port, const void *buf, unsigned long len)
{
	asm volatile ("rep;outsb" :: "S" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

static void
insb(unsigned short port, void *buf, unsigned long len)
{
	asm volatile ("rep;insb" :: "D" (buf), "c" (len),
			"d" (port) : "cc", "memory");
}

#endif

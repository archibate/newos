#ifndef _KERN_ASM_CMOS_H
#define _KERN_ASM_CMOS_H

#include <sys/io.h>

#define CMOS	0x70

#define	NVRAM_BEG	0x0e	/* start of NVRAM: offset 14 */
#define	NVRAM_SIZE	50	/* 50 bytes of NVRAM */
/* NVRAM bytes 7 & 8: base memory size */
#define NVRAM_BASEMEM	(NVRAM_BEG + 7)
/* NVRAM bytes 9 & 10: extended memory size */
#define NVRAM_EXTMEM	(NVRAM_BEG + 9)
/* NVRAM bytes 34 & 35: extended memory POSTed size */
#define NVRAM_PEXTMEM	(NVRAM_BEG + 34)
/* NVRAM byte 36: current century.  (please increment in Dec99!) */
#define NVRAM_CENTURY	(NVRAM_BEG + 36)

static int
cmos_read(int i)
{
	outb(CMOS+0, i);
	return inb(CMOS+1);
}

static void
cmos_write(int i, int x)
{
	outb(CMOS+0, i);
	outb(CMOS+1, x);
}

static int
nvram_read(int r)
{
	return cmos_read(r) | (cmos_read(r + 1) << 8);
}

#endif

#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/asm/ide.h>
#include <sys/io.h>

#define PHYS_BSIZE  		512
#define PBPB    (BSIZE/PHYS_BSIZE)

static int ide_wait(void)
{
	int timeout = 40000;
	int r;

	while ((r = inb(IDE_STAT)) & IDE_BSY) {
		if (timeout-- <= 0) {
			printk("WARNING: IDE wait timed out");
			return 0;
		}
	}

	if (r & (IDE_DF|IDE_ERR)) {
		printk("WARNING: IDE error %#x", r);
		return 0;
	}

	return 1;
}

static void ide_seek(int ide, int sectnr, int nsects)
{
	if (!ide_wait())
		panic("ide_seek: IDE wait error");

	int lba = sectnr;

	outb(IDE_SECTNR, nsects);

	outb(IDE_LBA0, lba         & 0xff);
	outb(IDE_LBA1, (lba >> 8)  & 0xff);
	outb(IDE_LBA2, (lba >> 16) & 0xff);

	unsigned char cur = 0xe0;
	cur |= (ide & 0x1) << 4;
	cur |= (lba >> 24) & 0x0f;
	outb(IDE_CURR, cur);
}

void ll_rw_block(struct buf *b, int rw)
{
	ide_seek(b->b_dev - 1, (b->b_blkno - 1) * PBPB, PBPB);
	if (rw == READ)
		outb(IDE_CMD, PBPB == 1 ? IDE_CMD_READ : IDE_CMD_RDMUL);
	else if (rw == WRITE)
		outb(IDE_CMD, PBPB == 1 ? IDE_CMD_WRITE : IDE_CMD_WRMUL);
	else
		panic("bad disk rw command");

	if (!ide_wait())
		panic("ll_rw_block: seek error: rw=%d, blkno=%d", rw, b->b_blkno);
	if (rw == READ) {
		b->b_uptodate = 1;
		insl(IDE_DAT, b->b_data, BSIZE/4);
	} else if (rw == WRITE) {
		b->b_dirt = 0;
		outsl(IDE_DAT, b->b_data, BSIZE/4);
	}
}

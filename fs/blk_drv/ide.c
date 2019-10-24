#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/asm/ide.h>
#include <sys/io.h>

#define PHYS_BLOCK_SIZE  		512
#define PBPB    (BLOCK_SIZE/PHYS_BLOCK_SIZE)

static void ide_wait(void)
{
	int timeout = 40000;
	int r;

	while ((r = inb(IDE_STAT)) & IDE_BSY) {
		if (timeout-- <= 0)
			panic("ide_wait timeout");
	}

	if (r & (IDE_DF|IDE_ERR))
		panic("ide_wait got error");
}

static void ide_seek(int ide, int sectnr, int nsects)
{
	ide_wait();

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
	printk("ll_rw_block: rw=%d, blkno=%d", rw, b->b_blkno);
	ide_seek(0, b->b_blkno - 1, PBPB);
	if (rw == READ)
		outb(IDE_CMD, PBPB == 1 ? IDE_CMD_READ : IDE_CMD_RDMUL);
	else if (rw == WRITE)
		outb(IDE_CMD, PBPB == 1 ? IDE_CMD_WRITE : IDE_CMD_WRMUL);
	else
		panic("bad disk rw command");

	ide_wait();
	if (rw == READ) {
		b->b_uptodate = 1;
		insl(IDE_DAT, b->b_data, BLOCK_SIZE/4);
	} else if (rw == WRITE) {
		b->b_dirt = 0;
		outsl(IDE_DAT, b->b_data, BLOCK_SIZE/4);
	}
}

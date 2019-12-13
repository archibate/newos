#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <string.h>

struct buf buffer[NBUFS];
static struct task *buffer_wait;

#ifdef _KDEBUG
void dump_buffer(int more)
{
	struct buf *b;
	printk("dev|blkno|udb|%");
	for (b = buffer; b < buffer + NBUFS; b++) {
		if (!b->b_dev) continue;
		if (!more && !b->b_count) continue;
		printk("%3d|%5d|%c%c%c|%1d",
		b->b_dev, b->b_blkno, "u-"[!b->b_uptodate],
		"d-"[!b->b_dirt], "b-"[!b->b_wait],
		b->b_count);
	}
}
#endif

static struct buf *getblk(dev_t dev, blkno_t blkno)
{
	struct buf *b, *eb = NULL;
again:
	for (b = buffer; b < buffer + NBUFS; b++) {
		if (b->b_dev == dev && b->b_blkno == blkno) {
			b->b_count++;
			return b;
		}
	}
#define BADNESS(b) ((b)->b_dirt * 2 + (b)->b_uptodate)
	for (b = buffer; b < buffer + NBUFS; b++)
		if (b->b_count == 0 &&
		   (!eb || BADNESS(b) < BADNESS(eb))) {
			eb = b;
			if (!BADNESS(eb))
				break;
		}
	if (!eb) {
		block_on(&buffer_wait);
		goto again;
	}
	b = eb;
	b->b_count = 1;
	b->b_dirt = 0;
	b->b_uptodate = 0;
	b->b_dev = dev;
	b->b_blkno = blkno;
	return b;
}

struct buf *bread(dev_t dev, blkno_t blkno)
{
	struct buf *b = getblk(dev, blkno);
	if (b->b_uptodate)
		return b;
	ll_rw_block(b, READ);
	while (!b->b_uptodate)
		block_on(&b->b_wait);
	return b;
}

void bwrite(struct buf *b)
{
	b->b_dirt = 1;
}

void brelse(struct buf *b)
{
	if (b->b_count <= 0)
		panic("trying to free free buffer");
	if (--b->b_count <= 0)
		wake_up(&buffer_wait);
}

// two easy-for-use APIs
void blk_readitem(dev_t dev, blkno_t blkno, size_t index, void *buf, size_t size)
{
	if (BSIZE % size)
		panic("blk_readitem: size not aligned");
	struct buf *b = bread(dev, blkno + (index * size) / BSIZE);
	memcpy(buf, b->b_data + (index * size) % BSIZE, size);
	brelse(b);
}

void blk_writeitem(dev_t dev, blkno_t blkno, size_t index, const void *buf, size_t size)
{
	if (BSIZE % size)
		panic("blk_writeitem: size not aligned");
	struct buf *b = bread(dev, blkno + (index * size) / BSIZE);
	memcpy(b->b_data + (index * size) % BSIZE, buf, size);
	bwrite(b);
	brelse(b);
}

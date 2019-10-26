#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <string.h>

struct buf buffer[NBUFS];
static struct task *buffer_wait;

static struct buf *getblk(int blkno)
{
	struct buf *b;
again:
	for (b = buffer; b < buffer + NBUFS; b++) {
		if (b->b_blkno == blkno) {
			b->b_count++;
			return b;
		}
	}
	for (b = buffer; b < buffer + NBUFS; b++) {
		if (b->b_count == 0) {
			b->b_count = 1;
			b->b_dirt = 0;
			b->b_uptodate = 0;
			b->b_blkno = blkno;
			return b;
		}
	}
	block_on(&buffer_wait);
	goto again;
}

struct buf *bread(int blkno)
{
	struct buf *b = getblk(blkno);
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
	if (b->b_count-- <= 0)
		panic("trying to free free buffer");
	wake_up(&buffer_wait);
}

// two easy-for-use APIs
void blk_readitem(int blkno, size_t index, void *buf, size_t size)
{
	if (BLOCK_SIZE % size)
		panic("blk_readitem: size not aligned");
	struct buf *b = bread(blkno + (index * size) / BLOCK_SIZE);
	memcpy(buf, b->b_data + (index * size) % BLOCK_SIZE, size);
	brelse(b);
}

void blk_writeitem(int blkno, size_t index, const void *buf, size_t size)
{
	if (BLOCK_SIZE % size)
		panic("blk_writeitem: size not aligned");
	struct buf *b = bread(blkno + (index * size) / BLOCK_SIZE);
	memcpy(b->b_data + (index * size) % BLOCK_SIZE, buf, size);
	bwrite(b);
	brelse(b);
}

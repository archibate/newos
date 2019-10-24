#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>

struct buf buffer[NBUFS];
static struct task *buffer_wait;

static struct buf *find_buffer(int blkno)
{
	struct buf *b;
	for (b = buffer; b < buffer + NBUFS; b++)
		if (b->b_blkno == blkno)
			return b;
	return NULL;
}

static struct buf *getblk(int blkno)
{
	struct buf *b;
again:
	b = find_buffer(blkno);
	if (b) return b;
	for (b = buffer; b < buffer + NBUFS; b++) {
		if (!b->b_count) {
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
	block_on(&b->b_wait);
	if (b->b_uptodate)
		return b;
	brelse(b);
	return NULL;
}

void brelse(struct buf *b)
{
	block_on(&b->b_wait);
	if (b->b_count-- < 0)
		panic("trying to free free buffer");
	wake_up(&buffer_wait);
}

#ifndef _KERN_FS_H
#define _KERN_FS_H 1

// Get NULL, size_t.
#include <stddef.h>

#define BLOCK_SIZE	1024
#define NBUFS		64
#define READ		0
#define WRITE		1

struct buf {
	char b_data[BLOCK_SIZE];
	int b_count;
	int b_blkno;
	int b_uptodate;
	int b_dirt;
	struct task *b_wait;
};

extern struct buf buffer[NBUFS];

void ll_rw_block(struct buf *b, int rw);
struct buf *bread(int blkno);
void brelse(struct buf *b);

#endif

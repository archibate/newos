#ifndef _KERN_FS_H
#define _KERN_FS_H 1

#include <stddef.h>
#include <sys/types.h>
#include <kern/nefs.h>

#define BSIZE	1024
#define NBUFS	128
#define NINODES	128
#define READ	0
#define WRITE	1

struct buf {
	char b_data[BSIZE];
	int b_count;
	int b_blkno;
	int b_uptodate;
	int b_dirt;
	struct task *b_wait;
};

struct inode {
	struct nefs_inode i_nefs;
	int i_count;
	ino_t i_ino;
	int i_uptodate;
	int i_dirt;
};

#define i_mode i_nefs.mode
#define i_size i_nefs.size
#define i_nlink i_nefs.nlink
#define i_zone i_nefs.zone
#define i_s_zone i_nefs.s_zone
#define i_atime i_nefs.atime

extern struct buf buffer[NBUFS];
struct inode inodes[NINODES];

void ll_rw_block(struct buf *b, int rw);
struct buf *bread(int blkno);
void bwrite(struct buf *b);
void brelse(struct buf *b);
void blk_readitem(int blkno, size_t index, void *buf, size_t size);
void blk_writeitem(int blkno, size_t index, const void *buf, size_t size);
struct nefs_super_block *get_super(void);
struct inode *idup(struct inode *ip);
void iput(struct inode *ip);
struct inode *iget(int ino);
size_t rw_inode(int rw, struct inode *ip, size_t pos, void *buf, size_t size);
size_t iread(struct inode *ip, size_t pos, void *buf, size_t size);
size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size);

#endif
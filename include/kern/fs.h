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

#define S_CHECK(mode, access) ((mode) & ((access) << 6))

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

#define super_block nefs_super_block
#define dir_entry nefs_dir_entry

#define i_mode i_nefs.i_nefs_mode
#define i_size i_nefs.i_nefs_size
#define i_nlink i_nefs.i_nefs_nlink
#define i_zone i_nefs.i_nefs_zone
#define i_s_zone i_nefs.i_nefs_s_zone
#define i_atime i_nefs.i_nefs_atime

extern struct buf buffer[NBUFS];
struct inode inodes[NINODES];

void ll_rw_block(struct buf *b, int rw);
struct buf *bread(int blkno);
void bwrite(struct buf *b);
void brelse(struct buf *b);
void blk_readitem(int blkno, size_t index, void *buf, size_t size);
void blk_writeitem(int blkno, size_t index, const void *buf, size_t size);
struct inode *create_inode(struct inode *pip);
struct super_block *get_super(void);
struct inode *idup(struct inode *ip);
void iupdate(struct inode *ip);
void iput(struct inode *ip);
struct inode *iget(ino_t ino);
size_t rw_inode(int rw, struct inode *ip, size_t pos, void *buf, size_t size);
size_t iread(struct inode *ip, size_t pos, void *buf, size_t size);
size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size);
int dir_read_entry(struct inode *dir, struct nefs_dir_entry *de, int i);
struct inode *namei(const char *path);
struct inode *creati(const char *path, int excl, mode_t mode);

#endif

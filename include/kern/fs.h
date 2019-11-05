#ifndef _KERN_FS_H
#define _KERN_FS_H 1

#include <stddef.h>
#include <sys/types.h>
#include <kip/nefs.h>
#include <bits/fcntl.h>
#include <bits/unistd.h>
#include <ds/list.h>

#define BSIZE	1024
#define NBUFS	128
#define NINODES	256
#define NFILES	256
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

struct file {
	struct inode *f_ip;
	off_t f_offset;
	int f_flags;
	int f_fdargs;
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
extern struct inode inodes[NINODES];
extern struct file files[NFILES];

// blk_drv
void ll_rw_block(struct buf *b, int rw);
// chr_drv
size_t chr_drv_rw(int rw, int nr, off_t pos, void *buf, size_t size);
// buffer.c
struct buf *bread(int blkno);
void bwrite(struct buf *b);
void brelse(struct buf *b);
void blk_readitem(int blkno, size_t index, void *buf, size_t size);
void blk_writeitem(int blkno, size_t index, const void *buf, size_t size);
// super.c
struct super_block *get_super(void);
// inode.c
struct inode *create_inode(struct inode *pip);
struct inode *idup(struct inode *ip);
void iupdate(struct inode *ip);
void iput(struct inode *ip);
struct inode *iget(ino_t ino);
size_t rw_inode(int rw, struct inode *ip, size_t pos, void *buf, size_t size);
size_t iread(struct inode *ip, size_t pos, void *buf, size_t size);
size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size);
// namei.c
int dir_read_entry(struct inode *dir, struct nefs_dir_entry *de, int i);
struct inode *namei(const char *path);
struct inode *creati(const char *path, int excl, mode_t mode, int nod);
int linki(const char *path, struct inode *ip);
int unlinki(const char *path, int rmdir);
// file.c
struct file *fs_open(const char *path, int flags, mode_t mode);
struct file *fs_dup(struct file *f);
size_t fs_read(struct file *f, void *buf, size_t size);
size_t fs_write(struct file *f, const void *buf, size_t size);
off_t fs_seek(struct file *f, off_t offset, int whence);
void fs_close(struct file *f);

#endif

#ifndef _KERN_FS_H
#define _KERN_FS_H 1

#include <stddef.h>
#include <sys/types.h>
#include <kip/nefs.h>
#include <bits/dirent.h>
#include <bits/fcntl.h>
#include <bits/unistd.h>
#include <bits/stat.h>
#include <ds/ring.h>

#define DEV_NULL	0x000
#define DEV_ZERO	0x001
#define DEV_FB0		0x002
#define DEV_TTY0	0x100

#define DRV_HDA		1
#define DRV_HDB		2

#define ROOT_INO	NEFS_ROOT_INO
#define ROOT_DEV	DRV_HDA

#define SYMLOOP_MAX	8
#define NAME_MAX	NEFS_NAME_MAX
#define PATH_MAX	1024
#define PIPE_SIZE	4096

#define BSIZE		1024
#define NBUFS		128
#define NINODES		256
#define NR_SUPER	8

#define READ		0
#define WRITE		1

#define S_ISNOD(mode) (S_ISCHR(mode) || S_ISBLK(mode) \
		    || S_ISFIFO(mode) || S_ISSOCK(mode))

typedef unsigned int blkno_t;

struct buf {
	char b_data[BSIZE];
	int b_count;
	dev_t b_dev;
	blkno_t b_blkno;
	int b_uptodate;
	int b_dirt;
	struct task *b_wait;
};

struct pipe_inode {
	ring_t(char) p_ring;
	struct task *p_read_wait;
	struct task *p_write_wait;
};

#define IFS_NEFS 0
#define IFS_PIPE 1

struct inode {
	union {
		int i_nodnr;
		struct nefs_inode i_nefs;
		struct pipe_inode i_pipe;
	};
	dev_t i_dev;
	ino_t i_ino;
	int i_fstype;
	int i_count;
	int i_uptodate;
	int i_dirt;
	struct super_block *i_mount;
	//struct pipe_inode i_pipe;
};

#define i_mode i_nefs.i_nefs_mode
#define i_size i_nefs.i_nefs_size
#define i_nlink i_nefs.i_nefs_nlink
#define i_zone i_nefs.i_nefs_zone
#define i_s_zone i_nefs.i_nefs_s_zone
#define i_atime i_nefs.i_nefs_atime
#define i_p_ring i_pipe.p_ring
#define i_p_read_wait i_pipe.p_read_wait
#define i_p_write_wait i_pipe.p_write_wait

struct super_block {
	struct nefs_super_block s_nefs;
	dev_t s_dev;
	struct inode *s_imount;
};

#define s_magic s_nefs.s_nefs_magic
#define s_super_len s_nefs.s_nefs_super_len
#define s_blksize_log2 s_nefs.s_nefs_blksize_log2
#define s_imap_begblk s_nefs.s_nefs_imap_begblk
#define s_zmap_begblk s_nefs.s_nefs_zmap_begblk
#define s_itab_begblk s_nefs.s_nefs_itab_begblk
#define s_data_begblk s_nefs.s_nefs_data_begblk
#define s_imap_blknr s_nefs.s_nefs_imap_blknr
#define s_zmap_blknr s_nefs.s_nefs_zmap_blknr
#define s_itab_blknr s_nefs.s_nefs_itab_blknr
#define s_data_blknr s_nefs.s_nefs_data_blknr

#define FT_INODE	0
#define FT_MSGQ		1
struct file {
	union {
		struct inode *f_ip;
		void *f_ptr;
	};
	off_t f_offset;
	int f_type;
	int f_flags;
	int f_fdargs;
};

#define dir_entry nefs_dir_entry

extern struct buf buffer[NBUFS];
extern struct inode inodes[NINODES];
extern struct super_block super[NR_SUPER];

// blk_drv/
void ll_rw_block(struct buf *b, int rw);
// chr_drv/
size_t chr_drv_rw(int rw, int nr, size_t pos, void *buf, size_t size);
ssize_t chr_drv_seek(int nr, size_t pos);
int chr_drv_ioctl(int nr, int req, long arg);
// buffer.c
struct buf *bread(dev_t dev, blkno_t blkno);
void bwrite(struct buf *b);
void brelse(struct buf *b);
void blk_readitem(dev_t dev, blkno_t blkno, size_t index, void *buf, size_t size);
void blk_writeitem(dev_t dev, blkno_t blkno, size_t index, const void *buf, size_t size);
void dump_buffer(int more);
// super.c
struct super_block *get_super(dev_t dev);
struct super_block *load_super(dev_t dev);
int mount_super(struct super_block *sb, struct inode *ip);
int do_mount(dev_t dev, struct inode *ip);
int umount_super(struct super_block *sb);
int do_umount(struct inode *ip);
int unload_super(dev_t dev);
void dump_super(void);
// inode.c
struct inode *create_inode(struct inode *pip);
struct inode *idup(struct inode *ip);
void iupdate(struct inode *ip);
void iput(struct inode *ip);
struct inode *iget(dev_t dev, ino_t ino);
size_t rw_inode(int rw, struct inode *ip, size_t pos, void *buf, size_t size);
size_t iread(struct inode *ip, size_t pos, void *buf, size_t size);
size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size);
int istat(struct inode *ip, struct stat *st);
int iioctl(struct inode *ip, int req, long arg);
int iaccess(struct inode *ip, mode_t amode, int eacc);
ssize_t iseek(struct inode *ip, size_t size);
struct inode *alloc_m_inode(void);
void dump_inode(int more);
// namei.c
int dir_read_entry(struct inode *dir, struct nefs_dir_entry *de, int i);
struct inode *namei(const char *path);
struct inode *creati(const char *path, int excl, mode_t mode, int nod);
int linki(const char *path, struct inode *ip);
int unlinki(const char *path, int rmdir);
void follow_policy_enter(int nofollow, int symlink_nofollow);
void follow_policy_leave(void);
// file.c
struct file *fs_open(const char *path, int flags, mode_t mode);
struct file *fs_open_object(void *ptr, int type, int flags);
struct file *fs_dup(struct file *f);
size_t fs_read(struct file *f, void *buf, size_t size);
size_t fs_write(struct file *f, const void *buf, size_t size);
off_t fs_seek(struct file *f, off_t offset, int whence);
void fs_close(struct file *f);
int fs_dirread(struct file *f, struct dirent *de);
int fs_ioctl(struct file *f, int req, long arg);
int fs_pipe(struct file *fs[2]);
int fs_truncate_s(struct file *f, size_t length);
// pipe.c
struct inode *make_pipe(void);
size_t pipe_read(struct inode *ip, void *buf, size_t size);
size_t pipe_write(struct inode *ip, const void *buf, size_t size);
void close_pipe(struct inode *ip);
void free_pipe(struct inode *ip);
// sys.c
int open_fd_object(void *ptr, int type, int flags);
void *get_fd_object(int fd, int type);
void remove_fd_object(int fd);

#endif

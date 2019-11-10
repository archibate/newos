#include <kern/fs.h>
#include <kern/sched.h>
#include <kern/kernel.h>
#include <string.h>
#include <errno.h>

struct inode inodes[NINODES];
static struct task *inode_buffer_wait;

#ifdef _KDEBUG
void dump_inode(int more)
{
	struct inode *ip;
	printk("dev| ino |#l|ud|%");
	for (ip = inodes; ip < inodes + NINODES; ip++) {
		if (!ip->i_dev) continue;
		if (!more && !ip->i_count) continue;
		printk("%3d|%5d|%2d|%c%c|%d",
		ip->i_dev, ip->i_ino, ip->i_nlink,
		"u-"[!ip->i_uptodate], "d-"[!ip->i_dirt],
		ip->i_count);
	}
}
#endif

static struct inode *get_inode(dev_t dev, ino_t ino)
{
	struct inode *ip, *eip = NULL;
again:
	for (ip = inodes; ip < inodes + NINODES; ip++)
		if (ip->i_dev == dev && ip->i_ino == ino)
			return idup(ip);
#define BADNESS(ip) ((ip)->i_dirt * 2 + (ip)->i_uptodate)
	for (ip = inodes; ip < inodes + NINODES; ip++)
		if (ip->i_count == 0 &&
		   (!eip || BADNESS(ip) < BADNESS(eip))) {
			eip = ip;
			if (!BADNESS(eip))
				break;
		}
	if (!eip) {
		block_on(&inode_buffer_wait);
		goto again;
	}
	ip = eip;
	ip->i_count = 1;
	ip->i_dirt = 0;
	ip->i_uptodate = 0;
	ip->i_dev = dev;
	ip->i_ino = ino;
	return ip;
}

struct inode *idup(struct inode *ip)
{
	if (!ip)
		panic("idup(NULL) from %p -> %p",
				__builtin_return_address(1),
				__builtin_return_address(0));
	/*if (ip->i_ino == 41)
	printk("idup from %p -> %p -> %p -> %p: ++c=%d",
			__builtin_return_address(3),
			__builtin_return_address(2),
			__builtin_return_address(1),
			__builtin_return_address(0),
			ip->i_count + 1);*/
	ip->i_count++;
	return ip;
}

static void erase_inode(struct inode *ip)
{
	char c = 0;
	struct super_block *sb = get_super(ip->i_dev);
	blk_readitem(ip->i_dev, sb->s_imap_begblk, ip->i_ino / 8, &c, 1);
	c &= ~(1 << ip->i_ino % 8);
	blk_writeitem(ip->i_dev, sb->s_imap_begblk, ip->i_ino / 8, &c, 1);
}

void iput(struct inode *ip)
{
	if (!ip)
		panic("iput(NULL) from %p -> %p",
				__builtin_return_address(1),
				__builtin_return_address(0));
	/*if (ip->i_ino == 41)
	printk("iput from %p -> %p -> %p: --c=%d",
			__builtin_return_address(2),
			__builtin_return_address(1),
			__builtin_return_address(0),
			ip->i_count - 1);*/
	if (ip->i_count <= 0)
		panic("trying to free free inode");
	if (--ip->i_count <= 0) {
		wake_up(&inode_buffer_wait);
		if (ip->i_nlink <= 0)
			erase_inode(ip);
	}
}

void iupdate(struct inode *ip)
{
	ip->i_dirt = 1;
}

static int check_inode_exist(dev_t dev, ino_t ino)
{
	char c = 0;
	struct super_block *sb = get_super(dev);
	blk_readitem(dev, sb->s_imap_begblk, ino / 8, &c, 1);
	return !!(c & 1 << ino % 8);
}

static ino_t alloc_inode(struct inode *pip)
{
	struct super_block *sb = get_super(pip->i_dev);
	for (size_t t = 0; t < sb->s_imap_blknr; t++) {
		struct buf *b = bread(pip->i_dev, sb->s_imap_begblk + t);
		for (size_t i = 0; i < BSIZE; i++) {
			for (size_t j = 0; j < 8; j++) {
				if (!(b->b_data[i] & 1 << j)) {
					b->b_data[i] |= 1 << j;
					bwrite(b);
					brelse(b);
					return i * 8 + j;
				}
			}
		}
		brelse(b);
	}
	panic("file system out of inode");
}

static int alloc_zone(struct inode *ip)
{
	struct super_block *sb = get_super(ip->i_dev);
	for (size_t t = 0; t < sb->s_zmap_blknr; t++) {
		struct buf *b = bread(ip->i_dev, sb->s_zmap_begblk + t);
		for (size_t i = 0; i < BSIZE; i++) {
			for (int j = 0; j < 8; j++) {
				if (!(b->b_data[i] & 1 << j)) {
					b->b_data[i] |= 1 << j;
					bwrite(b);
					brelse(b);
					return i * 8 + j;
				}
			}
		}
		brelse(b);
	}
	panic("file system out of zone");
}

static void load_inode(struct inode *ip)
{
	struct super_block *sb = get_super(ip->i_dev);
	blk_readitem(ip->i_dev, sb->s_itab_begblk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_uptodate = 1;
}

static void update_inode(struct inode *ip)
{
	struct super_block *sb = get_super(ip->i_dev);
	blk_writeitem(ip->i_dev, sb->s_itab_begblk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_dirt = 0;
}

struct inode *create_inode(struct inode *pip)
{
	ino_t ino = alloc_inode(pip);
	return iget(pip->i_dev, ino);
}

struct inode *iget(dev_t dev, ino_t ino)
{
	if (!check_inode_exist(dev, ino))
		return NULL;
	struct inode *ip = get_inode(dev, ino);
	if (ip->i_uptodate)
		return ip;
	load_inode(ip);
	return ip;
}

size_t rw_inode(int rw, struct inode *ip, size_t pos, void *buf, size_t size)
{
	if (!ip)
		panic("rw_inode(NULL) from %p -> %p",
				__builtin_return_address(1),
				__builtin_return_address(0));
	if (S_ISCHR(ip->i_mode))
		return chr_drv_rw(rw, ip->i_zone[0], pos, buf, size);

	if (pos > ip->i_size) {
		printk("WARNING: i%s: pos > ip->i_size",
				rw == READ ? "read" : "write");
		errno = EINVAL;
		return 0;
	}
	if (pos + size > ip->i_size) {
		if (rw == READ) {
			size = ip->i_size - pos;
		} else {
			ip->i_size = pos + size;
			iupdate(ip);
		}
	}
	struct super_block *sb = get_super(ip->i_dev);
	size_t offset = pos % BSIZE;
	size_t sz_left = size;
	struct buf *s_blk = NULL;
	size_t iz = pos / BSIZE;
	nefs_zone_t *zid;
	while (sz_left) {
		if (!s_blk && iz >= NEFS_NR_DIRECT) {
			if (!ip->i_s_zone)
				ip->i_s_zone = alloc_zone(ip);
			s_blk = bread(ip->i_dev, sb->s_data_begblk + ip->i_s_zone);
			iz -= NEFS_NR_DIRECT;
		}
		if (s_blk && iz >= BSIZE / 4) {
			errno = EFBIG;
			break;
		}
		size_t n = sz_left;
		if (n > BSIZE - offset)
			n = BSIZE - offset;
		if (!s_blk)
			zid = ip->i_zone + iz;
		else
			zid = (nefs_zone_t *)s_blk->b_data + iz;
		if (!*zid) {
			*zid = alloc_zone(ip);
			if (s_blk)
				bwrite(s_blk);
			else
				iupdate(ip);
		}
		struct buf *b = bread(ip->i_dev, sb->s_data_begblk + *zid);
		if (rw == READ) {
			memcpy(buf, b->b_data + offset, n);
		} else {
			memcpy(b->b_data + offset, buf, n);
			bwrite(b);
		}
		brelse(b);
		offset = 0;
		sz_left -= n;
		buf += n;
		iz += 1;
	}
	if (s_blk) brelse(s_blk);
	return size - sz_left;
}

size_t iread(struct inode *ip, size_t pos, void *buf, size_t size)
{
	if (!S_CHECK(ip->i_mode, S_IROTH)) {
		errno = EPERM;
		return 0;
	}
	return rw_inode(READ, ip, pos, buf, size);
}

size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size)
{
	if (!S_CHECK(ip->i_mode, S_IWOTH)) {
		errno = EPERM;
		return 0;
	}
	return rw_inode(WRITE, ip, pos, (void *)buf, size);
}

int istat(struct inode *ip, struct stat *st)
{
	memset(st, 0, sizeof(struct stat));
	st->st_dev = ip->i_dev;
	st->st_ino = ip->i_ino;
	st->st_mode = ip->i_mode;
	st->st_nlink = ip->i_nlink;
	if (S_ISNOD(ip->i_mode))
		st->st_rdev = ip->i_zone[0];
	st->st_size = ip->i_size;
	return 0;
}

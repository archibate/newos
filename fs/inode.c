#include <kern/fs.h>
#include <kern/sched.h>
#include <kern/kernel.h>
#include <string.h>

struct inode inodes[NINODES];
static struct task *inode_buffer_wait;

static struct inode *get_inode(ino_t ino)
{
	struct inode *ip, *eip = NULL;
again:
	for (ip = inodes; ip < inodes + NINODES; ip++)
		if (ip->i_ino == ino)
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
	ip->i_ino = ino;
	return ip;
}

struct inode *idup(struct inode *ip)
{
	if (!ip)
		panic("idup(NULL) from %p -> %p",
				__builtin_return_address(1),
				__builtin_return_address(0));
	ip->i_count++;
	return ip;
}

void iput(struct inode *ip)
{
	if (!ip)
		panic("iput(NULL) from %p -> %p",
				__builtin_return_address(1),
				__builtin_return_address(0));
	if (ip->i_count <= 0)
		panic("trying to free free inode");
	if (--ip->i_count <= 0)
		wake_up(&inode_buffer_wait);
}

void iupdate(struct inode *ip)
{
	ip->i_dirt = 1;
}

static int check_inode_exist(ino_t ino)
{
	char c = 0;
	struct super_block *sb = get_super();
	blk_readitem(sb->s_imap_begblk, ino / 8, &c, 1);
	return !!(c & 1 << ino % 8);
}

static ino_t alloc_inode(struct inode *pip)
{
	struct super_block *sb = get_super();
	for (size_t t = 0; t < sb->s_imap_blknr; t++) {
		struct buf *b = bread(sb->s_imap_begblk + t);
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
	struct super_block *sb = get_super();
	for (size_t t = 0; t < sb->s_zmap_blknr; t++) {
		struct buf *b = bread(sb->s_zmap_begblk + t);
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
	struct super_block *sb = get_super();
	blk_readitem(sb->s_itab_begblk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_uptodate = 1;
}

static void update_inode(struct inode *ip)
{
	struct super_block *sb = get_super();
	blk_writeitem(sb->s_itab_begblk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_dirt = 0;
}

struct inode *create_inode(struct inode *pip)
{
	ino_t ino = alloc_inode(pip);
	return iget(ino);
}

struct inode *iget(ino_t ino)
{
	if (!check_inode_exist(ino))
		return NULL;
	struct inode *ip = get_inode(ino);
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
	struct super_block *sb = get_super();
	size_t offset = pos % BSIZE;
	size_t sz_left = size;
	struct buf *s_blk = NULL;
	size_t iz = pos / BSIZE;
	nefs_zone_t *zid;
	while (sz_left) {
		if (!s_blk && iz >= NEFS_NR_DIRECT) {
			if (!ip->i_s_zone)
				ip->i_s_zone = alloc_zone(ip);
			s_blk = bread(sb->s_data_begblk + ip->i_s_zone);
			iz -= NEFS_NR_DIRECT;
		}
		if (s_blk && iz >= BSIZE / 4) {
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
		struct buf *b = bread(sb->s_data_begblk + *zid);
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
	if (!S_CHECK(ip->i_mode, S_IROTH))
		return 0;
	return rw_inode(READ, ip, pos, buf, size);
}

size_t iwrite(struct inode *ip, size_t pos, const void *buf, size_t size)
{
	if (!S_CHECK(ip->i_mode, S_IWOTH))
		return 0;
	return rw_inode(WRITE, ip, pos, (void *)buf, size);
}

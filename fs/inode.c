#include <kern/fs.h>
#include <kern/sched.h>
#include <kern/kernel.h>
#include <string.h>

struct inode inodes[NINODES];
static struct task *inode_buffer_wait;

static struct inode *get_inode(int ino)
{
	struct inode *ip;
again:
	for (ip = inodes; ip < inodes + NINODES; ip++)
		if (ip->i_ino == ino)
			return idup(ip);
	for (ip = inodes; ip < inodes + NINODES; ip++) {
		if (ip->i_count == 0) {
			ip->i_count = 1;
			ip->i_dirt = 0;
			ip->i_uptodate = 0;
			ip->i_ino = ino;
			return ip;
		}
	}
	block_on(&inode_buffer_wait);
	goto again;
}

struct inode *idup(struct inode *ip)
{
	ip->i_count++;
	return ip;
}

void iput(struct inode *ip)
{
	if (!ip->i_count-- <= 0)
		panic("trying to free free inode");
	wake_up(&inode_buffer_wait);
}

static int check_inode_exist(int ino)
{
	struct nefs_super_block *sb = get_super();
	char c = 0;
	blk_readitem(sb->s_imap_begin_blk, ino / 8, &c, 1);
	return !!(c & (1 << ino % 8));
}

static void load_inode(struct inode *ip)
{
	struct nefs_super_block *sb = get_super();
	blk_readitem(sb->s_itab_begin_blk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_uptodate = 1;
}

static void update_inode(struct inode *ip)
{
	struct nefs_super_block *sb = get_super();
	blk_writeitem(sb->s_itab_begin_blk, ip->i_ino,
			&ip->i_nefs, NEFS_INODE_SIZE);
	ip->i_dirt = 0;
}

struct inode *iget(int ino)
{
	if (!check_inode_exist(ino))
		return NULL;
	struct inode *ip = get_inode(ino);
	if (ip->i_uptodate)
		return ip;
	load_inode(ip);
	return ip;
}

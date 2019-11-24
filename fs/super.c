#include <kern/fs.h>
#include <kern/kernel.h>
#include <string.h>

struct super_block super[NR_SUPER];

#ifdef _KDEBUG
void dump_super(void)
{
	panic("dump_super() UIMP");
}
#endif

struct super_block *get_super(dev_t dev)
{
	struct super_block *sb;
	for (sb = super; sb < super + NR_SUPER; sb++) {
		if (sb->s_dev == dev)
			return sb;
	}
	panic("file system not loaded dev=%d", dev);
}

struct super_block *load_super(dev_t dev)
{
	struct super_block *sb;
	for (sb = super; sb < super + NR_SUPER; sb++) {
		if (sb->s_dev == dev)
			return sb;
	}
	for (sb = super; sb < super + NR_SUPER; sb++) {
		if (!sb->s_dev)
			break;
	}
	if (sb == super + NR_SUPER)
		panic("too much super loaded");

	struct buf *b = bread(dev, NEFS_SUPER_BLKNO);
	memcpy(sb, b->b_data, sizeof(struct nefs_super_block));
	brelse(b);
	if (sb->s_magic != NEFS_MAGIC)
		panic("file system not recognized (dev=%d)", dev);
	if (sb->s_blksize_log2 != 10)
		panic("block size other than 1024 not supported (dev=%d)", dev);
	sb->s_dev = dev;
	sb->s_imount = NULL;
	printk("%.*s: %d inodes, %d blocks",
			BSIZE - sb->s_super_len,
			b->b_data + sb->s_super_len,
			sb->s_imap_blknr * BSIZE * 8,
			sb->s_zmap_blknr * BSIZE * 8);
	return sb;
}

int mount_super(struct super_block *sb, struct inode *ip)
{
	if (ip->i_mount)
		return -1;
	if (sb->s_imount)
		return -1;
	sb->s_imount = idup(ip);
	ip->i_mount = sb;
	return 0;
}

int do_mount(dev_t dev, struct inode *ip)
{
	struct super_block *sb = load_super(dev);
	if (!sb)
		return -1;
	return mount_super(sb, ip);
}

int umount_super(struct super_block *sb)
{
	if (!sb->s_imount)
		return -1;
	sb->s_imount->i_mount = NULL;
	iput(sb->s_imount);
	sb->s_imount = NULL;
	return 0;
}

int do_umount(struct inode *ip)
{
	if (!ip->i_mount)
		return -1;
	return umount_super(ip->i_mount);
}

int unload_super(dev_t dev)
{
	struct super_block *sb = get_super(dev);
	if (sb->s_imount)
		return 0;
	sb->s_dev = 0;
	return -1;
}

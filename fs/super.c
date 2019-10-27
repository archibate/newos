#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <string.h>

static struct super_block sb;

struct super_block *get_super(void)
{
	if (sb.s_magic != NEFS_MAGIC)
		panic("file system not loaded");
	return &sb;
}

void fs_init(void)
{
	struct buf *b = bread(2);
	memcpy(&sb, b->b_data, sizeof(sb));
	brelse(b);
	if (sb.s_magic != NEFS_MAGIC)
		panic("file system not recognized");
	if (sb.s_blksize_log2 != 10)
		panic("block size other than 1024 not supported");
	printk("%.*s: %d inodes, %d blocks",
			BSIZE - sb.s_super_len,
			b->b_data + sb.s_super_len,
			sb.s_imap_blknr * BSIZE * 8,
			sb.s_zmap_blknr * BSIZE * 8);
	current->root = iget(NEFS_ROOT_INO);
	current->cwd = namei("/root");
}

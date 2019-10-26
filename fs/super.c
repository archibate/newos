#include <kern/fs.h>
#include <kern/kernel.h>
#include <string.h>
#include <malloc.h>

struct nefs_super_block *get_super(void)
{
	static struct nefs_super_block *sb;
	if (!sb) {
		struct buf *b = bread(2);
		sb = malloc(sizeof(*sb));
		memcpy(sb, b->b_data, sizeof(*sb));
		brelse(b);
		if (sb->s_magic != NEFS_MAGIC)
			panic("file system not recognized");
		if (sb->s_blksize_log2 != 10)
			panic("block size other than 1024 not supported");
		printk("%.*s: %d inodes, %d blocks",
				BLOCK_SIZE - sb->s_super_len,
				b->b_data + sb->s_super_len,
				sb->s_imap_blknr * BLOCK_SIZE * 8,
				sb->s_zmap_blknr * BLOCK_SIZE * 8);
	}
	return sb;
}

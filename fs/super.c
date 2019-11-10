#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <kern/tty.h>
#include <string.h>

static struct super_block sb;

struct super_block *get_super(dev_t dev)
{
	if (dev != ROOT_DEV)
		panic("bad device number %d", dev);
	if (sb.s_magic != NEFS_MAGIC)
		panic("file system not loaded");
	return &sb;
}

void load_super(dev_t dev)
{
	struct buf *b = bread(dev, NEFS_SUPER_BLKNO);
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
}

void fs_init(void)
{
	load_super(ROOT_DEV);

	current->root = iget(ROOT_DEV, ROOT_INO);
	current->cwd = namei("/root");

	iput(creati("/dev/tty", 1, S_IFDIR | 0755, 0));
	iput(creati("/dev/tty/mux", 1, S_IFCHR | 0644, TTY_MUX));
	iput(creati("/dev/tty/com0", 1, S_IFCHR | 0644, TTY_COM0));
	iput(creati("/dev/tty/vga", 1, S_IFCHR | 0644, TTY_VGA));
	iput(creati("/dev/hda", 1, S_IFBLK | 0644, DEV_HDA));
	iput(creati("/dev/hdb", 1, S_IFBLK | 0644, DEV_HDB));
}

#ifdef _KDEBUG
void dump_super(void)
{
	panic("dump_super() UIMP");
}
#endif

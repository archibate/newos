#include <kern/fs.h>
#include <kern/sched.h>
#include <kern/tty.h>

void fs_init(void)
{
	load_super(ROOT_DEV);

	current->root = iget(ROOT_DEV, ROOT_INO);
	current->cwd = namei("/root");

	iput(creati("/dev/tty", 1, S_IFDIR | 0755, 0));
	iput(creati("/dev/tty/mux", 1, S_IFCHR | 0644, DEV_TTY0 + TTY_MUX));
	iput(creati("/dev/tty/com0", 1, S_IFCHR | 0644, DEV_TTY0 + TTY_COM0));
	iput(creati("/dev/tty/vga", 1, S_IFCHR | 0644, DEV_TTY0 + TTY_VGA));
	iput(creati("/dev/null", 1, S_IFCHR | 0644, DEV_NULL));
	iput(creati("/dev/zero", 1, S_IFCHR | 0644, DEV_ZERO));
	iput(creati("/dev/hda", 1, S_IFBLK | 0644, DRV_HDA));
	iput(creati("/dev/hdb", 1, S_IFBLK | 0644, DRV_HDB));

	do_mount(DRV_HDB, namei("/mnt"));
}

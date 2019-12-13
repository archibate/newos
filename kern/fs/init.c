#include <kern/fs.h>
#include <kern/sched.h>
#include <kern/tty.h>

void fs_init(void)
{
	load_super(ROOT_DEV);

	current->root = iget(ROOT_DEV, ROOT_INO);
	current->cwd = namei("/root");

	iput(creati("/dev/tty", 1, S_IFCHR | 0660, DEV_TTY0 + TTY_STD));
	iput(creati("/dev/ttyS0", 1, S_IFCHR | 0660, DEV_TTY0 + TTY_COM0));
	iput(creati("/dev/ttyS1", 1, S_IFCHR | 0660, DEV_TTY0 + TTY_VGA));
	iput(creati("/dev/null", 1, S_IFCHR | 0666, DEV_NULL));
	iput(creati("/dev/zero", 1, S_IFCHR | 0666, DEV_ZERO));
	iput(creati("/dev/hda", 1, S_IFBLK | 0660, DRV_HDA));
	iput(creati("/dev/hdb", 1, S_IFBLK | 0660, DRV_HDB));
#ifdef _VIDEO
	iput(creati("/dev/fb0", 1, S_IFCHR | 0660, DEV_FB0));
	iput(creati("/dev/mouse", 1, S_IFCHR | 0440, DEV_MOUSE));
	iput(creati("/dev/keybd", 1, S_IFCHR | 0440, DEV_TTY0 + TTY_VGA));
#endif
	//do_mount(DRV_HDB, namei("/mnt"));
}

#include <kern/kernel.h>
#include <kern/sched.h>
#include <kern/fs.h>
#include <kern/mm.h>
#include <bits/kdebug.h>
#include <errno.h>

#ifdef _KDEBUG
int sys_kdebug(int cmd, long arg)
{
	switch (cmd) {
	case KDB_TEST:
		break;
	case KDB_PRINTINT:
		printk("[%d] %d", current->pid, arg);
		break;
	case KDB_PRINTSTR:
		printk("[%d] %s", current->pid, (const char *)arg);
		break;
	case KDB_HALT:
		panic("[%d] halted by KDB", current->pid);
		break;
	case KDB_DUMP_INODE:
		dump_inode(arg);
		break;
	case KDB_DUMP_SUPER:
		dump_super();
		break;
	case KDB_DUMP_BUFFER:
		dump_buffer(arg);
		break;
	case KDB_DUMP_KHEAP:
		dump_kernel_heap(arg);
		break;
	case KDB_DUMP_TASKS:
		dump_tasks();
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}
#else
int sys_kdebug(int cmd, long arg)
{
	errno = ENOSYS;
	return -1;
}
#endif

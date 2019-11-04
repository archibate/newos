#include <kern/sched.h>
#include <kern/exec.h>
#include <kern/kernel.h>
#include <kern/mm.h>
#include <string.h>

#define FS_DUP(f) ((f) ? fs_dup(f) : NULL)

static void do_fork(struct task *child, struct task *parent)
{
	child->mm = mm_fork(parent->mm);
	memcpy(task_regs(child), task_regs(parent), REGS_SIZE);

	task_regs(child)[EAX] = 0;
	child->filp[0] = FS_DUP(parent->filp[0]);
	child->filp[1] = FS_DUP(parent->filp[1]);
	child->filp[2] = FS_DUP(parent->filp[2]);
}

int fork_start(void *unused)
{
	move_to_user();
}

pid_t sys_fork(void)
{
	struct task *child;
	child = kernel_thread(fork_start, NULL);
	do_fork(child, current);
	return child->pid;
}

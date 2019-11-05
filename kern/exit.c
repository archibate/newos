#include <kern/sched.h>
#include <kern/kernel.h>
#include <bits/wait.h>
#include <kern/fs.h>
#include <kern/mm.h>
#include <malloc.h>
#include <errno.h>

static void free_task(struct task *p)
{
	free_kernel_stack(p->stack);
}

static void tell_parent(int pid)
{
	if (pid) {
		for (int i = 0; i < NTASKS; i++) {
			if (!task[i] || task[i]->pid != pid)
				continue;
			task[i]->signal |= SIG(SIGCHLD);
			return;
		}
	}
	panic("no parent ppid=%d found", pid);
}

__attribute__((noreturn)) void
sys_exit(int status)
{
	for (int i = 0; i < NR_OPEN; i++)
		if (current->filp[i])
			fs_close(current->filp[i]);
	if (current->cwd) iput(current->cwd);
	current->cwd = NULL;
	if (current->root) iput(current->root);
	current->root = NULL;
	if (current->mm) free_mm(current->mm);
	current->mm = NULL;

	current->state = TASK_ZOMBIE;
	current->exit_stat = _MAKE_WSTAT(status);
	tell_parent(current->ppid);
	schedule();
	panic("sys_exit schedule does return");
}

int sys_waitpid(pid_t pid, int *stat_loc, int options)
{
	int flag;
repeat:
	flag = 0;
	for (int i = NTASKS - 1; i > 0; i--) {
		if (!task[i] || task[i] == current)
			continue;
		if (task[i]->ppid != current->pid)
			continue;
		if (pid > 0) {
			if (task[i]->pid != pid)
				continue;
		}
		if (task[i]->state == TASK_ZOMBIE) {
			flag = task[i]->pid;
			*stat_loc = task[i]->exit_stat;
			free_task(task[i]);
			task[i] = NULL;
			return flag;
		} else {
			flag = 1;
		}
	}
	if (flag) {
		if (options & WNOHANG)
			return 0;
		current->state = TASK_SLEEPING;
		schedule();
		if (!(current->signal &= ~SIG(SIGCHLD)))
			goto repeat;
		errno = EINTR;
		return -1;
	}
	errno = ECHILD;
	return -1;
}

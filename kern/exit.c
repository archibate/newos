#include <kern/sched.h>
#include <kern/kernel.h>
#include <bits/wait.h>
#include <kern/fs.h>
#include <kern/mm.h>
#include <malloc.h>
#include <errno.h>

static void free_task(struct task *p)
{
	if (p->command) free(p->command);
	free_kernel_stack(p->stack);
}

static void tell_parent(int pid)
{
	if (pid) {
		for (int i = 0; i < NTASKS; i++) {
			if (!task[i] || task[i]->pid != pid)
				continue;
			task[i]->signal |= _S(SIGCHLD);
			return;
		}
	}
	//printk("WARNING: no parent ppid=%d found", pid);
	return;
}

__attribute__((noreturn)) void
do_exit(int exit_code)
{
	/*printk("do_exit %d %#x from usrip:%p\n", current->pid, exit_code,
			1[(void**)0[(void**)task_regs(current)[EBP]]]);*/
	if (current->pid == 0)
		panic("idle task trying to exit");
	if (current->pid == 1)
		panic("init task exited with %#x", exit_code);
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
	current->exit_code = exit_code;
	tell_parent(current->ppid);
	schedule();
	panic("sys_exit schedule does return (%d %d)",
			current->pid, current->state);
}

__attribute__((noreturn)) void
sys_exit(int status)
{
	do_exit((status & 0xff) << 8);
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
		if (task[i]->state == TASK_STOPPED) {
			if (!(options & WUNTRACED))
				continue;
			*stat_loc = 0x7fff;
			return task[i]->pid;
		} else if (task[i]->state == TASK_ZOMBIE) {
			flag = task[i]->pid;
			if (stat_loc)
				*stat_loc = task[i]->exit_code;
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
		if (!(current->signal &= ~_S(SIGCHLD)))
			goto repeat;
		errno = EINTR;
		return -1;
	}
	errno = ECHILD;
	return -1;
}

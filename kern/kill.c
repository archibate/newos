#include <kern/sched.h>
#include <errno.h>

int do_kill(struct task *p, int sig, long arg)
{
	p->signal |= _S(sig);
	p->siginfo[sig] = arg;
	if (p == current)
		check_signal();
	return 0;
}

int sys_kill_a(pid_t pid, int sig, long arg)
{
	if ((unsigned)sig-1 >= _NSIG) {
		errno = EINVAL;
		return -1;
	}
	if (pid > 0) {
		for (int i = NTASKS - 1; i > 0; i--) {
			if (task[i] && task[i]->pid == pid)
				return do_kill(task[i], sig, arg);
		}
	}
	errno = ESRCH;
	return -1;
}

int sys_raise_a(int sig, long arg)
{
	if ((unsigned)sig > _NSIG) {
		errno = EINVAL;
		return -1;
	}
	return do_kill(current, sig, arg);
}

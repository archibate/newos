#include <kern/sched.h>
#include <kern/kernel.h>

long sys_sgetmask(void)
{
	return current->blocked;
}

long sys_ssetmask(long newmask)
{
	long old = current->blocked;
	current->blocked = newmask & _BLOCKABLE;
	return old;
}

void *sys_signal_r(int sig, void *handler, void *restorer)
{
	if ((unsigned)sig-1 >= _NSIG || sig == SIGKILL)
		return SIG_ERR;
	struct sigaction *sa = &current->sigact[sig-1];
	void *old = sa->sa_handler;
	sa->sa_handler = handler;
	sa->sa_restorer = restorer;
	sa->sa_mask = 0;
	sa->sa_flags = SA_ONESHOT | SA_NOMASK;
	return old;
}

static void do_signal(int sig)
{
	reg_t *regs = task_regs(current);
	struct sigaction *sa = &current->sigact[sig-1];
	current->signal &= ~_S(sig);
	if (sa->sa_handler == SIG_IGN)
		return;
	if (sa->sa_handler == SIG_DFL) {
		if (sig == SIGCHLD)
			return;
		else
			do_exit(0xffff | ((sig & 0xff) << 16));
	}

	reg_t *sp = (reg_t *)regs[ESP];
	*--sp = regs[EIP];
	*--sp = regs[EFLAGS];
	*--sp = regs[EDX];
	*--sp = regs[ECX];
	*--sp = regs[EAX];
	*--sp = (sa->sa_flags & SA_NOMASK) ? 0 : current->blocked;
	*--sp = sig;
	*--sp = (reg_t)sa->sa_restorer;
	regs[ESP] = (reg_t)sp;
	regs[EIP] = (reg_t)sa->sa_handler;
	current->blocked |= sa->sa_mask;
	if (sa->sa_flags & SA_ONESHOT)
		sa->sa_handler = SIG_DFL;
	current->interrupted = 1;
}

void check_signal(void)
{
	int sig;
	if (!current->mm)
		return;
#if 0
	if (current->state == TASK_SLEEPING)
		current->state = 0;
	else if (current->state != 0)
		return;
#endif
	sigset_t signal = task_signal(current);
	for (sig = 1; signal && sig <= _NSIG; sig++) {
		if (signal & 1) {
			do_signal(sig);
			return;
		}
		signal >>= 1;
	}
}

#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/fs.h>
#include <malloc.h>
#include <string.h>
#include <stddef.h>

struct task *task[NTASKS];
struct task *current;

__attribute__((fastcall)) void switch_context(
		unsigned long *prev, unsigned long *next);

void
switch_to(int i)
{
	struct mm *previous_mm;
	struct task *previous;
	if (current != task[i]) {
		previous = current;
		current = task[i];
		switch_context(previous->kregs, current->kregs);
	}
}

void
schedule(void)
{
#if 0
	// (DEP signal)
	for (int i = NTASKS - 1; i >= 0; i--) {
		struct task *p = task[i];
		if (p && p->state == TASK_SLEEPING && p->signal)
			p->state = 0;
	}
#endif

	int next;
	while (1) {
		next = 0;
		int c = -1;
		for (int i = NTASKS - 1; i >= 0; i--) {
			struct task *p = task[i];
			if (p && p->state == 0 && p->counter > c)
				c = p->counter, next = i;
		}
		if (c) break;
		for (int i = NTASKS - 1; i >= 0; i--) {
			struct task *p = task[i];
			if (p)
				p->counter = (p->counter >> 1) + p->priority;
		}
	}
	switch_to(next);
}

void
sched_timer_callback(void)
{
	if (current->counter > 0)
		current->counter--;
	schedule();
}

void
do_pause(void)
{
	current->state = TASK_SLEEPING;
	schedule();
}

void
block_on(struct task **p)
{
	struct task *old = *p;
	*p = current;
	current->state = TASK_BLOCKED;
	schedule();
	if (old) old->state = 0;
}

void
sleep_on(struct task **p)
{
	struct task *old = *p;
	*p = current;
repeat:	current->state = TASK_SLEEPING;
	schedule();
	if (*p && *p != current) {
		(*p)->state = 0;
		goto repeat;
	}
	*p = NULL;
	if (old) old->state = 0;
}

void
wake_up(struct task **p)
{
	if (!*p) return;
	(*p)->state = 0;
	*p = NULL;
}

int
get_pid_index(pid_t pid)
{
	for (int i = 0; i < NTASKS; i++)
		if (task[i]->pid == pid)
			return i;
	return -1;
}

static struct task initial_task;

void
sched_init(void)
{
	initial_task.priority = 1;
	current = task[0] = &initial_task;
}

static int last_pid = 0;

static int
find_empty_task(void)
{
repeat:
	last_pid++;
	for (int i = 0; i < NTASKS; i++) {
		if (task[i] && task[i]->pid == last_pid)
			goto repeat;
	}
	for (int i = 1; i < NTASKS; i++) {
		if (!task[i])
			return i;
	}
	panic("failed to find empty task");
}

struct task *
new_task(struct task *parent)
{
	int i = find_empty_task();
	struct task *p = calloc(1, sizeof(struct task));
	task[i] = p;
	p->ppid = parent->pid;
	p->pid = last_pid;
	p->priority = parent->priority;
	p->root = idup(parent->root);
	p->cwd = idup(parent->cwd);
	p->stack = malloc(STACK_SIZE);
	return p;
}

void del_task(struct task *p)
{
	iput(p->root);
	iput(p->cwd);
	free(p->stack);
	memset(p, 0, sizeof(struct task));
}

__attribute__((noreturn)) void
kthread_exit(int status)
{
	current->state = TASK_ZOMBIE;
	schedule();
	panic("kthread_exit schedule returned");
}

__attribute__((noreturn)) static void
__kthread_exit(void)
{
	register int ret asm ("eax");
	kthread_exit(ret);
}

struct task *
kernel_thread(void *start, void *arg)
{
	struct task *p = new_task(current);
	void **sp = p->stack + STACK_SIZE;
	*--sp = arg;
	*--sp = __kthread_exit;
	*--sp = start;
	p->kregs[K_ESP] = (unsigned long) sp;
	return p;
}

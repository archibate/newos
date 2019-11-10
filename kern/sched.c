#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/tss.h>
#include <kern/mm.h>
#include <kern/fs.h>
#include <malloc.h>
#include <string.h>
#include <stddef.h>

struct task *task[NTASKS];
struct task *current;

__attribute__((fastcall)) void switch_context(
		reg_t *prev, reg_t *next);

#define TASK_RUNNING	0
#define TASK_SLEEPING	1
#define TASK_BLOCKED	2
#define TASK_ZOMBIE	3
#define TASK_STOPPED	4
#ifdef _KDEBUG
void dump_tasks(void)
{
	printk(" pid|ppid|prio|s| command");
	for (int i = 0; i < NTASKS; i++) {
		struct task *p = task[i];
		if (!p) continue;
		printk("%4d|%4d|%4d|%c| %s", p->pid, p->ppid,
			p->priority, "rwbzs?????"[p->state % 10],
			p->command ? p->command : "(noname)");
	}
}
#endif

static void switch_to_current_from(struct task *previous)
{
	if (current->mm) {
		switch_to_mm(current->mm);
		check_signal();
	}

	tss0.ts_esp0 = (unsigned long)(current->stack + STACK_SIZE);
	switch_context(previous->kregs, current->kregs);
}

void
switch_to(int i)
{
	struct task *previous;
	if (current != task[i]) {
		previous = current;
		current = task[i];
		switch_to_current_from(previous);
	}
}

void
schedule(void)
{
	for (int i = NTASKS - 1; i > 0; i--) {
		struct task *p = task[i];
		if (p && p->state == TASK_SLEEPING && task_signal(p))
			p->state = 0;
	}

	int next;
	while (1) {
		next = 0;
		int c = -1;
		for (int i = NTASKS - 1; i > 0; i--) {
			struct task *p = task[i];
			if (p && p->state == 0 && p->counter > c)
				c = p->counter, next = i;
		}
		if (c) break;
		for (int i = NTASKS - 1; i > 0; i--) {
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

int
sys_pause(void)
{
	current->state = TASK_SLEEPING;
	schedule();
	return 0;
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
		if (task[i] && task[i]->pid == pid)
			return i;
	return -1;
}

static struct task initial_task;

void
sched_init(void)
{
	initial_task.priority = 0;
	initial_task.command = strdup("(idle)");
	current = task[0] = &initial_task;
	extern char boot_stack_top[];
	current->stack = boot_stack_top - STACK_SIZE;
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
	p->stack = alloc_kernel_stack();
	return p;
}

__attribute__((noreturn)) static void
__sys_exit(void)
{
	register int ret asm ("eax");
	sys_exit(ret);
}

struct task *
kernel_thread(void *start, void *arg)
{
	struct task *p = new_task(current);
	void **sp = p->stack + STACK_SIZE - REGS_SIZE - 4;
	*--sp = arg;
	*--sp = __sys_exit;
	*--sp = start;
	p->kregs[K_ESP] = (unsigned long) sp;
	return p;
}

pid_t sys_getpid(void)
{
	return current->pid;
}

pid_t sys_getppid(void)
{
	return current->ppid;
}

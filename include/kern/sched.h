#pragma once

// Get pid_t.
#include <sys/types.h>
// Get register indexes.
#include <sys/reg.h>
// Get signal indexes.
#include <bits/signal.h>

#define SIG(sig) (1 << ((sig) - 1))

#define TASK_RUNNING	0
#define TASK_SLEEPING	1
#define TASK_BLOCKED	2
#define TASK_ZOMBIE	3
#define TASK_STOPPED	4

#define NR_OPEN		64
#define STACK_SIZE	8192

#define K_ESP		0
#define K_EBX		1	
#define K_ESI		2
#define K_EDI		3
#define K_EBP		4
#define K_EFLAGS	5
#define NR_KREGS	6

struct task {
	int state;
	int counter;
	int priority;
	pid_t pid, ppid;
	unsigned long kregs[NR_KREGS];
	void *stack;

	int exit_stat;
	int signal;

	struct inode *cwd;
	struct inode *root;
	struct mm_struct *mm;
	struct file *filp[NR_OPEN];
};

#define task_regs(p) ((reg_t *)((p)->stack + STACK_SIZE - REGS_SIZE))

#define NTASKS	64

extern struct task *task[NTASKS];
extern struct task *current;
extern struct task *exit_wait;

void switch_to(int i);
void schedule(void);
void do_pause(void);
void block_on(struct task **p);
void sleep_on(struct task **p);
void wake_up(struct task **p);
int get_pid_index(pid_t pid);
struct task *new_task(struct task *parent);
struct task *kernel_thread(void *start, void *arg);
__attribute__((noreturn)) void sys_exit(int status);
void sched_timer_callback(void);

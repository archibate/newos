#pragma once

// Get pid_t.
#include <sys/types.h>
// Get register indexes.
#include <sys/reg.h>
// Get signal indexes.
#include <bits/signal.h>
// Get CLOCKS_PER_SEC, time_t, clock_t.
#include <bits/time.h>

#define _S(sig) (1 << ((sig) - 1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

#define TASK_RUNNING	0
#define TASK_SLEEPING	1
#define TASK_BLOCKED	2
#define TASK_ZOMBIE	3
#define TASK_STOPPED	4

#define MAX_PRIORITY	10
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
	reg_t kregs[NR_KREGS];
	void *stack;
	clock_t alarm;

	int exit_code;
	sigset_t signal, blocked;
	struct sigaction sigact[_NSIG];

	struct inode *cwd;
	struct inode *root;
	struct mm_struct *mm;
	struct file *filp[NR_OPEN];

	char *command;
};

#define task_regs(p) ((reg_t *)((p)->stack + STACK_SIZE - REGS_SIZE))
#define task_signal(p) ((p)->signal & ~(_BLOCKABLE & (p)->blocked))

#define NTASKS	64

extern clock_t jiffies;
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
__attribute__((noreturn)) void do_exit(int exit_code);
int do_kill(struct task *p, int sig);
int sys_kill(pid_t pid, int sig);
int sys_raise(int sig);
void sched_timer_callback(void);
void check_signal(void);
void dump_tasks(void);

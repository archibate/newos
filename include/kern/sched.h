#ifndef _KERN_SCHED_H
#define _KERN_SCHED_H 1

// Get pid_t.
#include <sys/types.h>
// Get register indexes.
#include <sys/reg.h>
// Get signal indexes.
#include <bits/signal.h>
// Get CLOCKS_PER_SEC, time_t, clock_t.
#include <bits/time.h>
// >>>>>>>>>>>> IPC
// Get list data structure.
#include <ds/list.h>
// <<<<<<<<<<<< IPC

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

#if 0
struct handle {
	struct list_node list;
	struct task *server;
};

struct container {
	struct list_head handles;
};
#endif

// >>>>>>>>>>>> IPC
#define NR_CONN NR_OPEN
#define NR_CHANNELS 8

enum endpoint_status
{
	EP_FREE = 0,	/* recver busy on other things and no sender, buffer clean */
	EP_WAIT,	/* recv -> send: recver ready, buffer clean */
	EP_SENT,	/* send -> recv: sender ready, buffer valid */
	EP_RCVD,	/* recv -> reply: recver processing, ep->reply valid */
};

struct endpoint
{
	enum endpoint_status state;

	struct list_head sender;	/* queue waiting for you to be READY */
	struct connection *reply;	/* target you accepted & going to reply */
	struct task *server;		/* endpoint owner */

	char *buf;		/* message recving buffer */
	size_t bufsize;		/* buffer size */
	size_t size;		/* size of actual message */
};
// <<<<<<<<<<<< IPC

struct connection
{
	struct endpoint *endp;	/* connect to what */
	struct task *client;	/* connection owner */
};

struct task {
	int state;
	int counter;
	int priority;
	pid_t pid, ppid;
	reg_t kregs[NR_KREGS];
	void *stack;
	clock_t alarm;

	sigset_t signal, blocked;
	struct sigaction sigact[_NSIG];
	long siginfo[_NSIG];
	int exit_code;
	int kr_interrupted;
	int ks_nowait;

	struct inode *cwd;
	struct inode *root;
	struct mm_struct *mm;
	struct file *filp[NR_OPEN];

// >>>>>>>>>>>> IPC
	struct list_node ipc_list;
	struct connection conn[NR_CONN];
	struct endpoint channels[NR_CHANNELS];
// <<<<<<<<<<<< IPC

#if 0
	struct container *conp[NR_CONT];
#endif

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
int schedule(void);
int sys_pause(void);
void block_policy_enter(int o_nonblock);
void block_policy_leave(void);
void block_on(struct task **p);
int sleep_on(struct task **p);
void wake_up(struct task **p);
int get_pid_index(pid_t pid);
struct task *new_task(struct task *parent);
struct task *kernel_thread(void *start, void *arg);
__attribute__((noreturn)) void sys_exit(int status);
__attribute__((noreturn)) void do_exit(int exit_code);
int do_kill(struct task *p, int sig, long arg);
int sys_kill_a(pid_t pid, int sig, long arg);
int sys_raise_a(int sig, long arg);
void sched_timer_callback(void);
void check_signal(void);
void dump_tasks(void);

#endif

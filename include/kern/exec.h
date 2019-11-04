#ifndef _KERN_EXEC_H
#define _KERN_EXEC_H 1

#include <kern/mm.h>
#include <kern/fs.h>
#include <kern/sched.h>

#define USER_STACK_SIZE (1024*4096)

int mm_load_exec(struct mm_struct *mm, reg_t *regs, struct inode *ip);
int do_execve(struct inode *ip);
__attribute__((noreturn)) static void move_to_user(void)
{
	__attribute__((noreturn)) extern void __move_to_user(reg_t *regs);
	__move_to_user(task_regs(current));
}

#endif

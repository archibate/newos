#include <kern/exec.h>
#include <kern/kernel.h>
#include <string.h>

int do_execve(struct inode *ip)
{
	struct mm_struct *mm = create_mm();
	if (mm_load_exec(mm, task_regs(current), ip) == -1)
		return -1;
	if (current->mm)
		free_mm(current->mm);
	switch_to_mm(current->mm = mm);
	return 0;
}

int sys_execve(const char *path)
{
	struct inode *ip = namei(path);
	if (!ip) return -1;
	int ret = do_execve(ip);
	iput(ip);
	return ret;
}

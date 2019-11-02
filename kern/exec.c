#include <kern/exec.h>
#include <kern/kernel.h>
#include <string.h>

int do_execve(struct inode *ip)
{
	struct mm_struct *mm = create_mm();
	if (mm_load_exec(mm, ip) == -1)
		return -1;

	if (current->mm)
		free_mm(current->mm);
	switch_to_mm(current->mm = mm);
	return 0;
}

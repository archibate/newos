#include <kern/exec.h>

void *sys_sbrk(intptr_t incptr)
{
	viraddr_t old_ebrk = current->mm->ebrk, ebrk = old_ebrk + incptr;
	if (ebrk >= current->mm->stop || ebrk < current->mm->ebss)
		return (void *)-1;
	current->mm->ebrk = ebrk;
	return (void *)old_ebrk;
}

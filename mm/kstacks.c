#include <kern/mm.h>
#include <kern/sched.h>

static void *ks_top = (void *)KERNEL_STKS + PGSIZE;

void *alloc_kernel_stack(void)
{
	size_t i;
	for (i = 0; i < STACK_SIZE + PGSIZE - 1; i += PGSIZE)
		page_insert(mmu_get_pgdir(), alloc_page(), ks_top + i, PG_W);
	void *stack = ks_top;
	ks_top += i + PGSIZE;
	return stack;
}

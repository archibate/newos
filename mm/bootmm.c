#include <kern/mm.h>
#include <kern/kernel.h>
#include <string.h>

void *boot_alloc(size_t n)
{
	static void *kern_top;
	extern int ebss;
	if (!kern_top)
		kern_top = &ebss;
	void *p = PAGEUP(kern_top);
	kern_top = p + PAGEUP(n);
	memset(p, 0, n);
	return p;
}

pde_t *kern_pd;
pte_t *kern_ptes;

// initialized in memtest.c:
extern size_t total_pages;

void
bootmm_init(void)
{
	size_t npts = total_pages / 1024;
	kern_pd = boot_alloc(PGSIZE);
	kern_ptes = boot_alloc(npts * PGSIZE);
	pde_t pde = paddr(kern_ptes) | PG_P | PG_W;
	pte_t pte = 0x0 | PG_P | PG_W;
	for (int i = 0; i < npts; i++) {
		kern_pd[i] = pde;
		kern_pd[(KERNEL_BASE >> 22) + i] = pde;
		pde += PGSIZE;
		for (int j = 0; j < 1024; j++) {
			kern_ptes[i * 1024 + j] = pte;
			pte += PGSIZE;
		}
	}
	mmu_set_pgdir(kern_pd);
	mmu_enable(1);
}

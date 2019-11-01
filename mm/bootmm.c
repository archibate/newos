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

void switch_pgdir(pde_t *pd)
{
	memcpy(pd, mmu_get_pgdir(), (KERNEL_END >> 22) * sizeof(pde_t));
	mmu_set_pgdir(pd);
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
	for (size_t i = 0; i < npts; i++) {
		kern_pd[i] = pde;
		kern_pd[(KERNEL_BASE >> 22) + i] = pde;
		pde += PGSIZE;
		for (int j = 0; j < 1024; j++) {
			kern_ptes[i * 1024 + j] = pte;
			pte += PGSIZE;
		}
	}
	kern_ptes[0] = 0; // nuke NULL
	mmu_set_pgdir(kern_pd);
	mmu_enable(1);
}

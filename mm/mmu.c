#include <kern/mm.h>
#include <kern/kernel.h>
#include <sys/intrin.h>

void
mmu_enable(int enable)
{
	unsigned long cr0 = scr0();
	if (enable)
		cr0 |= 0x80010000;
	else
		cr0 &= 0x7fffffff;
	lcr0(cr0);
}

void
mmu_set_pgdir(pde_t *pd)
{
	lcr3(paddr(pd));
}

pde_t *
mmu_get_pgdir(void)
{
	return kvaddr(scr3());
}

void
tlb_invalidate(pde_t *pd, void *va)
{
	invlpg(va);
}

pte_t *
pgdir_walk(pde_t *pd, void *va, int create)
{
	unsigned long i = (long)va >> 22, j = ((long)va >> 12) & 0x3ff;
	if (!(pd[i] & PG_P)) {
		if (!create)
			return NULL;
		struct page_info *page = alloc_page_zero();
		pd[i] = page2pa(page) | PG_P | PG_W | PG_U;
	}
	return (pte_t *)kvaddr(pd[i] & PGMASK) + j;
}

struct page_info *
page_lookup(pde_t *pd, void *va, pte_t **pte_store)
{
	pte_t *pte = pgdir_walk(pd, va, 0);
	if (!pte) return NULL;
	if (pte_store) *pte_store = pte;
	return pa2page(*pte & PGMASK);
}

void
page_insert(pde_t *pd, struct page_info *page, void *va, int perm)
{
	pte_t *pte = pgdir_walk(pd, va, 1);
	if (*pte & PG_P) {
		//printk("WARNING: page_insert: twice map va=%p, pa=%p", va, *pte & PGMASK);
		put_page(pa2page(*pte & PGMASK));
	}
	*pte = page2pa(page) | perm | PG_P;
	tlb_invalidate(pd, va);
}

void
page_remove(pde_t *pd, void *va)
{
	pte_t *pte;
	struct page_info *page = page_lookup(pd, va, &pte);
	put_page(page);
	*pte = 0;
	tlb_invalidate(pd, va);
}

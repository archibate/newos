#ifndef _KERN_MM_H
#define _KERN_MM_H 1

// Get size_t.
#include <stddef.h>

#define PGSIZE	0x1000
#define PGMASK	0xfffff000
#define PG_P	0x1
#define PG_W	0x2
#define PG_U	0x4

#define KERNEL_BASE 0x0
#define KERNEL_CODE 0x100000
#define KERNEL_PMAP 0x400000
#define KERNEL_HEAP 0x20000000

#define kvaddr(pa) ((void *)(pa) + KERNEL_BASE)
#define paddr(va) ((physaddr_t)(va) - KERNEL_BASE)

typedef size_t physaddr_t, pte_t, pde_t;

struct page_info
{
	struct page_info *next;
	int refcnt;
};

void mmu_enable(int enable);
void mmu_set_pgdir(pde_t *pd);
void tlb_invalidate(pde_t *pd, void *va);
pte_t *pgdir_walk(pde_t *pd, void *va, int create);
struct page_info *page_lookup(pde_t *pd, void *va, pte_t **pte_store);
void page_insert(pde_t *pd, struct page_info *page, void *va, int perm);
void page_remove(pde_t *pd, void *va);

physaddr_t page2pa(struct page_info *page);
struct page_info *pa2page(physaddr_t pa);
struct page_info *alloc_page(void);
struct page_info *alloc_page_zero(void);
struct page_info *duplicate_page(struct page_info *page);
void put_page(struct page_info *page);
void *boot_alloc(size_t n);

static void *
page2kva(struct page_info *page)
{
	return kvaddr(page2pa(page));
}

static struct page_info *
kva2page(void *va)
{
	return pa2page(paddr(va));
}

extern size_t total_pages;
extern physaddr_t base_mem_end;
extern pde_t *kern_pd;
extern pte_t *kern_ptes;

#define PGDOWN(x) ((typeof(x))(((size_t)(x)) & PGMASK))
#define PAGEUP(x) ((typeof(x))(((size_t)(x) + PGSIZE - 1) & PGMASK))

#endif

#ifndef _KERN_MM_H
#define _KERN_MM_H 1

// Get size_t.
#include <stddef.h>
// Get off_t.
#include <sys/types.h>
// Get uintptr_t.
#include <stdint.h>
// Get list data structure.
#include <ds/list.h>

#define PGSIZE	0x1000
#define PGMASK	0xfffff000
#define PG_P	0x1
#define PG_W	0x2
#define PG_U	0x4

#define KERNEL_BASE 0x0
#define KERNEL_CODE 0x100000
#define KERNEL_PMAP 0x400000
#define KERNEL_STKS 0x18000000
#define KERNEL_HEAP 0x20000000
#define KERNEL_END  0x40000000

#define kvaddr(pa) ((void *)(pa) + KERNEL_BASE)
#define paddr(va) ((physaddr_t)(va) - KERNEL_BASE)

typedef uintptr_t physaddr_t, viraddr_t, pte_t, pde_t;

struct page_info
{
	struct page_info *next;
	int refcnt;
};

void mmu_enable(int enable);
pde_t *mmu_get_pgdir(void);
void mmu_set_pgdir(pde_t *pd);
void switch_pgdir(pde_t *pd);
void tlb_invalidate(pde_t *pd, void *va);
pte_t *pgdir_walk(pde_t *pd, void *va, int create);
struct page_info *page_lookup(pde_t *pd, void *va, pte_t **pte_store);
void page_insert(pde_t *pd, struct page_info *page, void *va, int perm);
void page_remove(pde_t *pd, void *va);

physaddr_t page2pa(struct page_info *page);
struct page_info *pa2page(physaddr_t pa);
struct page_info *alloc_page(void);
struct page_info *alloc_page_zero(void);
struct page_info *dup_page(struct page_info *page);
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

void *alloc_kernel_stack(void);

extern size_t total_pages;
extern physaddr_t base_mem_end;
extern pde_t *kern_pd;
extern pte_t *kern_ptes;

#define PGOFFS(x) ((uintptr_t)(x) & (PGSIZE - 1))
#define PGDOWN(x) ((typeof(x))(((uintptr_t)(x)) & PGMASK))
#define PAGEUP(x) ((typeof(x))(((uintptr_t)(x) + PGSIZE - 1) & PGMASK))

#define PROT_EXEC	1
#define PROT_WRITE	2
#define PROT_READ	4
#define PROT_NONE	0

#define MAP_SHARED	1
#define MAP_PRIVATE	2
#define MAP_FIXED	3

struct mm_struct
{
	struct list_head mm_areas;
	pde_t *pd;

	viraddr_t ebss, ebrk, stop;
};

struct vm_area_struct
{
	struct list_node vm_list;

	viraddr_t vm_begin, vm_end;
	int vm_prot, vm_flags;

	struct mm_struct *vm_mm;

	struct inode *vm_file;
	off_t vm_file_offset;

	struct list_head vm_pages;
};

struct vm_page
{
	struct list_node pg_list;

	physaddr_t pg_paddr;

	unsigned pg_index;
	struct vm_area_struct *pg_area;

	struct vm_page *cow_next;

	int pg_ready;
};

struct mm_struct *create_mm(void);
void free_mm(struct mm_struct *mm);
struct vm_area_struct *mm_find_area(
		struct mm_struct *mm,
		viraddr_t begin,
		viraddr_t end);
struct vm_area_struct *mm_new_area(
		struct mm_struct *mm,
		viraddr_t begin, size_t size,
		int prot, int flags,
		struct inode *file, off_t offset);
void mm_del_area(struct vm_area_struct *vm);
struct vm_page *vm_area_new_page(
		struct vm_area_struct *vm,
		unsigned index);
void vm_area_del_page(struct vm_page *pg);
struct mm_struct *mm_fork(
		struct mm_struct *mm);
static void switch_to_mm(
		struct mm_struct *mm)
{
	switch_pgdir(mm->pd);
}

#endif

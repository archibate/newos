#include <kern/mm.h>
#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <sys/intrin.h>
#include <sys/reg.h>
#include <string.h>
#include <malloc.h>

static int vm_mmu_perm(struct vm_area_struct *vm)
{
	int perm = PG_U;
	if (vm->vm_prot & PROT_WRITE)
		perm |= PG_W;
	return perm;
}

struct mm_struct *create_mm(void)
{
	struct mm_struct *mm = calloc(sizeof(struct mm_struct), 1);
	mm->pd = page2kva(alloc_page_zero());
	return mm;
}

void free_mm(struct mm_struct *mm)
{
	struct vm_area_struct *vm;
	put_page(kva2page(mm->pd));
	list_foreach(vm, &mm->mm_areas, vm_list)
		mm_del_area(vm);
	free(mm);
}

static struct vm_page *dup_mm_vm_page(
		struct vm_area_struct *vm2,
		struct vm_page *pg)
{
	struct vm_page *pg2;
	struct page_info *page2;
	pg2 = malloc(sizeof(struct vm_page));
	memcpy(pg2, pg, sizeof(struct vm_page));
	pg2->pg_list.pprev = NULL;
	pg2->pg_list.next = NULL;
	pg2->pg_area = vm2;
	page2 = alloc_page();
	memcpy(page2kva(page2), kvaddr(pg2->pg_paddr), PGSIZE);
	pg2->pg_paddr = page2pa(page2);
	viraddr_t va = vm2->vm_begin + (pg2->pg_index << 12);
	page_insert(vm2->vm_mm->pd, page2, (void *)va, vm_mmu_perm(vm2));
	return pg2;
}

static struct vm_area_struct *dup_mm_vm_area(
		struct mm_struct *mm2,
		struct vm_area_struct *vm)
{
	struct vm_area_struct *vm2;
	struct vm_page *pg, *pg2;
	struct list_node **pprev;
	vm2 = malloc(sizeof(struct vm_area_struct));
	memcpy(vm2, vm, sizeof(struct vm_area_struct));
	vm2->vm_list.pprev = NULL;
	vm2->vm_list.next = NULL;
	vm2->vm_pages = LIST_INIT;
	vm2->vm_file = vm->vm_file ? idup(vm->vm_file) : NULL;
	vm2->vm_mm = mm2;
	pprev = &vm2->vm_pages.first;
	list_foreach(pg, &vm->vm_pages, pg_list) {
		pg2 = dup_mm_vm_page(vm2, pg);
		*pprev = &pg2->pg_list;
		pg2->pg_list.pprev = pprev;
		pprev = &pg2->pg_list.next;
	}
	*pprev = NULL;
	return vm2;
}

struct mm_struct *mm_fork(
		struct mm_struct *mm)
{
	struct vm_area_struct *vm, *vm2;
	struct mm_struct *mm2 = create_mm();
	struct list_node **pprev = &mm2->mm_areas.first;
	list_foreach(vm, &mm->mm_areas, vm_list) {
		vm2 = dup_mm_vm_area(mm2, vm);
		*pprev = &vm2->vm_list;
		vm2->vm_list.pprev = pprev;
		pprev = &vm2->vm_list.next;
	}
	*pprev = NULL;
	return mm2;
}

struct vm_area_struct *mm_find_area(
		struct mm_struct *mm,
		viraddr_t begin,
		viraddr_t end)
{
	struct vm_area_struct *vm;
	list_foreach(vm, &mm->mm_areas, vm_list) {
		if (vm->vm_begin <= begin && vm->vm_end >= end)
			return vm;
	}
	return NULL;
}

struct vm_area_struct *mm_new_area(
		struct mm_struct *mm,
		viraddr_t begin, size_t size,
		int prot, int flags,
		struct inode *file, off_t offset)
{
	struct vm_area_struct *vm;
	viraddr_t end = begin + size - 1;
	vm = mm_find_area(mm, begin, end);
	if (vm) return NULL;
	vm = calloc(sizeof(struct vm_area_struct), 1);
	vm->vm_begin = begin;
	vm->vm_end = end;
	vm->vm_prot = prot;
	vm->vm_flags = flags;
	vm->vm_file = file ? idup(file) : NULL;
	vm->vm_file_offset = offset;
	vm->vm_mm = mm;
	list_insert_head(&vm->vm_list, &mm->mm_areas);
	return vm;
}

void mm_del_area(struct vm_area_struct *vm)
{
	struct vm_page *pg;
	if (vm->vm_file)
		iput(vm->vm_file);
	list_foreach(pg, &vm->vm_pages, pg_list)
		vm_area_del_page(pg);
	__list_remove(&vm->vm_list);
	free(vm);
}

struct vm_page *vm_area_new_page(
		struct vm_area_struct *vm,
		unsigned index)
{
	struct vm_page *pg;
	list_foreach(pg, &vm->vm_pages, pg_list)
		if (pg->pg_index == index)
			return pg;
	pg = calloc(sizeof(struct vm_page), 1);
	pg->pg_area = vm;
	pg->pg_paddr = page2pa(alloc_page());
	pg->pg_index = index;
	list_insert_head(&pg->pg_list, &vm->vm_pages);
	return pg;
}

void vm_area_del_page(struct vm_page *pg)
{
	struct vm_area_struct *vm = pg->pg_area;
	struct mm_struct *mm = vm->vm_mm;
	viraddr_t vaddr = vm->vm_begin + (pg->pg_index << 12);
	page_remove(mm->pd, (void *)vaddr);
	__list_remove(&pg->pg_list);
	free(pg);
}

int mm_page_fault(
		struct mm_struct *mm,
		viraddr_t va, int errcd)
{
	struct vm_page *pg;
	struct vm_area_struct *vm;
	vm = mm_find_area(mm, va, va);
	if (!vm) return 0;

	int index = (va - vm->vm_begin) >> 12;
	int perm = vm_mmu_perm(vm);
	if (errcd & ~perm) return 0;
	pg = vm_area_new_page(vm, index);
	struct page_info *page = dup_page(pa2page(pg->pg_paddr));
	if (vm->vm_file) {
		off_t offset = vm->vm_file_offset + (pg->pg_index << 12);
		iread(vm->vm_file, offset, page2kva(page), PGSIZE);
	} else {
		memset(page2kva(page), 0, PGSIZE);
	}
	page_insert(mm->pd, page, (void *)va, perm);
	return 1;
}

int do_page_fault(reg_t *regs)
{
	viraddr_t va = scr2();
	printk("do_page_fault %p %d", va, regs[ERRCODE]);
	return va >= KERNEL_END && mm_page_fault(current->mm, va, regs[ERRCODE]);
}

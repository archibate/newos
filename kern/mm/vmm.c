#include <kern/mm.h>
#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <sys/intrin.h>
#include <sys/reg.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

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
	struct vm_area_struct *vm, *_vmn;
	put_page(kva2page(mm->pd));
	list_foreach_s(_vmn, vm, &mm->mm_areas, vm_list)
		mm_del_area(vm);
	free(mm);
}

static void update_vm_page(struct vm_page *pg, int permask)
{
	viraddr_t va = pg->pg_area->vm_begin + (pg->pg_index << 12);
	page_insert(pg->pg_area->vm_mm->pd, dup_page(pa2page(pg->pg_paddr)),
			(void *)va, vm_mmu_perm(pg->pg_area) & ~permask);
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
	update_vm_page(pg2, PG_W);
	update_vm_page(pg, PG_W);
	pg2->cow_next = pg->cow_next;
	pg->cow_next = pg2;
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
	mm2->ebss = mm->ebss;
	mm2->ebrk = mm->ebrk;
	mm2->stop = mm->stop;
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

static void select_inn_vm_pages(
		struct vm_area_struct *vm1,
		struct vm_area_struct *vm)
{
	viraddr_t va;
	struct vm_page *pg, *pg1;
	struct list_node **pprev = &vm1->vm_pages.first;
	list_foreach(pg, &vm->vm_pages, pg_list) {
		va = vm->vm_begin + (pg->pg_index << 12);
		if (!(vm1->vm_begin <= va && va <= vm1->vm_end))
			continue;
		pg1 = dup_mm_vm_page(vm1, pg);
		pg1->pg_index = (va - vm1->vm_begin) >> 12;
		*pprev = &pg1->pg_list;
		pg1->pg_list.pprev = pprev;
		pprev = &pg1->pg_list.next;
	}
	*pprev = NULL;
}

static void mm_sep_del_area_in(
		struct vm_area_struct *vm,
		viraddr_t begin, viraddr_t end)
{
	struct vm_area_struct *vm1;
	if (vm->vm_begin < begin && vm->vm_end >= begin) {
		vm1 = malloc(sizeof(struct vm_area_struct));
		memcpy(vm1, vm, sizeof(struct vm_area_struct));
		vm1->vm_file = vm->vm_file ? idup(vm->vm_file) : NULL;
		vm1->vm_end = begin - 1;
		vm1->vm_pages = LIST_INIT;
		select_inn_vm_pages(vm1, vm);
		list_insert_head(&vm1->vm_list, &vm1->vm_mm->mm_areas);
	}
	if (vm->vm_begin <= end && vm->vm_end > end) {
		vm1 = malloc(sizeof(struct vm_area_struct));
		memcpy(vm1, vm, sizeof(struct vm_area_struct));
		vm1->vm_file = vm->vm_file ? idup(vm->vm_file) : NULL;
		vm1->vm_begin = end + 1;
		vm1->vm_file_offset += vm1->vm_begin - vm->vm_begin;
		vm1->vm_pages = LIST_INIT;
		select_inn_vm_pages(vm1, vm);
		list_insert_head(&vm1->vm_list, &vm1->vm_mm->mm_areas);
	}
	struct vm_area_struct *v;
	struct mm_struct *mm = vm->vm_mm;
	mm_del_area(vm);
}

int mm_find_sync_area(
		struct mm_struct *mm,
		viraddr_t begin, viraddr_t end)
{
	return 0;
}

int mm_find_replace_area(
		struct mm_struct *mm,
		viraddr_t begin, viraddr_t end,
		int noreplace)
{
	struct vm_area_struct *vm;
	vm = mm_find_area(mm, begin, end);
	if (!vm) return 1;
	if (noreplace) return 0;
	do {
		mm_sep_del_area_in(vm, begin, end);
	} while ((vm = mm_find_area(mm, begin, end)));
	return 1;
}

struct vm_area_struct *mm_new_area(
		struct mm_struct *mm,
		viraddr_t begin, size_t size,
		int prot, int flags,
		struct inode *file, off_t offset)
{
	struct vm_area_struct *vm;
	viraddr_t end = begin + size - 1;
	if (!mm_find_replace_area(mm, begin, end, flags & MAP_NOREPLACE)) {
		errno = EEXIST;
		return NULL;
	}
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
	struct vm_page *pg, *_pgn;
	if (vm->vm_file)
		iput(vm->vm_file);
	list_foreach_s(_pgn, pg, &vm->vm_pages, pg_list)
		vm_area_del_page(pg);
	list_remove(&vm->vm_list);
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
	pg->cow_next = pg;
	list_insert_head(&pg->pg_list, &vm->vm_pages);
	return pg;
}

static struct vm_page *get_cow_tail(struct vm_page *pg)
{
	int i;
	struct vm_page *tail;
	for (i = 5000, tail = pg; i && tail && tail->cow_next != pg;
			i--, tail = tail->cow_next);
	if (!i) panic("COW list too long / loop");
	if (!tail) panic("COW list tail pointed NULL");
	return tail;
}

void vm_area_del_page(struct vm_page *pg)
{
	struct vm_area_struct *vm = pg->pg_area;
	struct mm_struct *mm = vm->vm_mm;
	viraddr_t vaddr = vm->vm_begin + (pg->pg_index << 12);
	page_remove(mm->pd, (void *)vaddr);
	list_remove(&pg->pg_list);
	struct vm_page *tail = get_cow_tail(pg);
	tail->cow_next = pg->cow_next;
	pg->cow_next = NULL;
	free(pg);
}

static void do_cow_vm_page(struct vm_page *pg)
{
	struct vm_page *tail = get_cow_tail(pg);
	tail->cow_next = pg->cow_next;
	pg->cow_next = pg;

	struct page_info *page = pa2page(pg->pg_paddr);
	struct page_info *page2 = alloc_page();
	pg->pg_paddr = page2pa(page2);
	memcpy(page2kva(page2), page2kva(page), PGSIZE);
}

static void do_load_page(struct vm_page *pg)
{
	struct vm_area_struct *vm = pg->pg_area;
	if (vm->vm_file) {
		off_t offset = vm->vm_file_offset + (pg->pg_index << 12);
		iread(vm->vm_file, offset, kvaddr(pg->pg_paddr), PGSIZE);
	} else {
		memset(kvaddr(pg->pg_paddr), 0, PGSIZE);
	}
	pg->pg_ready = 1;
}

int mm_page_fault(
		struct mm_struct *mm,
		viraddr_t va, int errcd)
{
	struct vm_page *pg;
	struct vm_area_struct *vm;
	vm = mm_find_area(mm, va, va);
	if (!vm) return 0;
	int perm = vm_mmu_perm(vm);
	if ((errcd & (PG_W | PG_U)) & ~perm) return 0;

	pg = vm_area_new_page(vm, (va - vm->vm_begin) >> 12);
	if (!pg->pg_ready)
		do_load_page(pg);
	if ((perm & PG_W) && pg->cow_next != pg)
		do_cow_vm_page(pg);
	page_insert(mm->pd, dup_page(pa2page(pg->pg_paddr)), (void *)va, perm);
	return 1;
}

int do_page_fault(reg_t *regs)
{
	viraddr_t va = scr2();
	//printk("%d do_page_fault %p %d", current->pid, va, regs[ERRCODE]);
	return va >= KERNEL_END && mm_page_fault(current->mm, va, regs[ERRCODE]);
}

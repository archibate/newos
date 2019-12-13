#include <kern/mm.h>
#include <kern/kernel.h>
#include <string.h>

#define ALLOCATED_MAGIC ((void *)0xdeadc0de)

static struct page_info *page_free_head, *pages;
#define npages total_pages

physaddr_t
page2pa(struct page_info *page)
{
	size_t i = page - pages;
	if (i > npages) panic("bad struct page_info index %d", i);
	return i * PGSIZE;
}

struct page_info *
pa2page(physaddr_t pa)
{
	size_t i = pa >> 12;
	if (i > npages) panic("bad physical page address %p", pa);
	return &pages[i];
}

struct page_info *
alloc_page(void)
{
	if (!page_free_head)
		panic("out of memory");
	struct page_info *page = page_free_head;
	page_free_head = page->next;
	if (page_free_head == page)
		panic("loop page_free_list");
	page->next = ALLOCATED_MAGIC;
	page->refcnt = 1;
	return page;
}

struct page_info *
alloc_page_zero(void)
{
	struct page_info *page = alloc_page();
	memset((void *)page2kva(page), 0, PGSIZE);
	return page;
}

struct page_info *
dup_page(struct page_info *page)
{
	page->refcnt++;
	return page;
}

void
put_page(struct page_info *page)
{
	if (--page->refcnt > 0)
		return;
	if (page->next != ALLOCATED_MAGIC)
		panic("bad free_page pa=%p", page2pa(page));
	page->next = page_free_head;
	page_free_head = page;
}

void
pmm_init(void)
{
	pages = boot_alloc(sizeof(struct page_info) * npages);
	for (size_t i = KERNEL_PMAP >> 12; i < npages - 1; i++)
		pages[i].next = &pages[i + 1];
	pages[npages - 1].next = NULL;
	page_free_head = &pages[KERNEL_PMAP >> 12];
}

#include <stdlib.h>
#include <string.h>
#ifdef _KERNEL
#include <kern/kernel.h>
#include <kern/mm.h>

static void
edit_heap_range(void *begin, void *end) // FIXME: problem may come from this
{
	if (begin < end)
		for (void *p = PAGEUP(begin); p < PAGEUP(end); p += PGSIZE)
			page_insert(kern_pd, alloc_page(), p, PG_W);
	else if (end > begin)
		for (void *p = PAGEUP(end); p > PAGEUP(begin); p -= PGSIZE)
			page_remove(kern_pd, p);
}

static void *
sbrk(ptrdiff_t incptr)
{
	static void *pbrk;
	if (!pbrk) pbrk = (void *)KERNEL_HEAP;
	void *res = pbrk;
	edit_heap_range(pbrk, pbrk + incptr);
	pbrk += incptr;
	return res;
}

static int
brk(void *addr)
{
	return sbrk(addr - sbrk(0)) == (void *)-1 ? -1 : 0;
}
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define panic(...) do { \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
	abort(); \
} while (0)
#endif

typedef struct header
{
	struct header *next, *prev;
	size_t size;
	void *ptr;
} H;

static H *first_block;

static H *search_block(H **pprev, size_t size)
{
	*pprev = first_block;
	for (H *b = first_block; b; b = b->next) {
		if (!b->ptr && b->size >= size)
			return b;
		*pprev = b;
	}
	return NULL;
}

static H *extend_heap(H *prev, size_t size)
{
	H *b = sbrk(0);
	if (sbrk(sizeof(H) + size) == (void*)-1)
		return NULL;
	b->size = size;
	b->next = NULL;
	b->prev = prev;
	if (prev)
		prev->next = b;
	return b;
}

static void split_block(H *b, size_t size)
{
	H *c = (void*)(b + 1) + size;
	c->size = b->size - (size + sizeof(H));
	b->size = size;
	c->next = b->next;
	if (b->next)
		b->next->prev = c;
	c->prev = b;
	b->next = c;
	c->ptr = NULL;
}

void merge_block(H *b)
{
	if (b->next && !b->next->ptr) {
		b->size += sizeof(H) + b->next->size;
		b->next = b->next->next;
		if (b->next)
			b->next->prev = b;
	}
}

static size_t align(size_t s)
{
	return !(s & 7) ? s : ((s >> 3) + 1) << 3;
}

static void *_malloc(size_t size)
{
	H *b;
	size = align(size);
	if (!first_block) {
		b = extend_heap(NULL, size);
		if (!b)
			return NULL;
		first_block = b;
	} else {
		H *prev;
		b = search_block(&prev, size);
		if (!b) {
			b = extend_heap(prev, size);
			if (!b)
				return NULL;
		} else if (b->size >= size + sizeof(H) + 8) {
			/*if (b <= b->next && (void*)(b + 1) + b->size
			 > (void*)b->next)
				panic("%p+%p %p+%p", b, b->size,
				b->next, b->next->size);*/
			split_block(b, size);
			merge_block(b->next);
		}
	}
	b->ptr = b + 1;
	return b->ptr;
}

void *malloc(size_t size)
{
	void *p = _malloc(size);
	if (!p)
		return NULL;
	memset(p, 0xcc, size);
	return p;
}

void *calloc(size_t nmemb, size_t size)
{
	size *= nmemb;
	void *p = _malloc(size);
	if (!p)
		return NULL;
	memset(p, 0, size);
	return p;
}

static H *get_block(void *p)
{
	return p - sizeof(H);
}

void free(void *p)
{
	H *b = get_block(p);
	if (!(first_block && (H *)p >= first_block + 1
				&& p < sbrk(0) && p == b->ptr)) {
		if (p == b->ptr)
			panic("bad free %p < %p < %p", first_block + 1, p, sbrk(0));
		else
			panic("bad free %p (%p)", p, b->ptr);
	}
	//memset(p, 0xcc, b->size);

	b->ptr = NULL;
	if (b->prev && !b->prev->ptr) {
		b = b->prev;
		merge_block(b);
	}
	if (b->next)
		merge_block(b);
	else {
		if (b->prev)
			b->prev->next = NULL;
		else
			first_block = NULL;
		brk(b);
	}
}

void *realloc(void *p, size_t size)
{
	if (!p) return calloc(size, 1);
	H *b = get_block(p);
	void *q = _malloc(size);
	if (!q) return NULL;
	if (b->size < size) {
		memcpy(q, p, b->size);
		memset(q + b->size, 0, size - b->size);
	} else {
		memcpy(q, p, size);
	}
	free(p);
	return q;
}

#ifdef _KERNEL
#ifdef _KDEBUG
void dump_kernel_heap(int more)
{
	printk("a|   addr   | size ");
	for (H *node = first_block; node; node = node->next) {
		if (!more && !node->ptr) continue;
		printk("%c|%p|%4d", "a-B-"[!node->ptr + 2 * (node->ptr != node + 1)],
			node + 1, node->size, node->size);
	}
}
#endif
#endif

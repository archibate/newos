#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <malloc.h>

struct pipe *make_pipe(void)
{
	struct pipe *p = calloc(sizeof(struct pipe), 1);
	return p;
}

size_t pipe_read(struct pipe *p, void *buf, size_t size)
{
	char *b = buf;
	while (ring_empty(&p->p_ring))
		sleep_on(&p->p_read_wait);
	while (size && !ring_empty(&p->p_ring)) {
		ring_get(&p->p_ring, *b);
		b++; size--;
	}
	wake_up(&p->p_write_wait);
	return b - (char *)buf;
}

size_t pipe_write(struct pipe *p, const void *buf, size_t size)
{
	const char *b = buf;
	while (ring_full(&p->p_ring))
		sleep_on(&p->p_write_wait);
	while (size && !ring_full(&p->p_ring)) {
		ring_put(&p->p_ring, *b);
		b++; size--;
	}
	wake_up(&p->p_read_wait);
	return b - (char *)buf;
}

void free_pipe(struct pipe *p)
{
	free(p);
}

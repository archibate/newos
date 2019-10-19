#ifndef _KERN_RINGBUF_H
#define _KERN_RINGBUF_H 1

#include <stddef.h>

#define RINGBUF(type, size) \
	struct { \
		unsigned int head, tail; \
		type buf[size]; \
	}

#define RING_TYPE(r) typeof((r)->buf[0])
#define RING_SIZE(r) array_sizeof((r)->buf)
#define RING_CAPACITY(r) (RING_SIZE(r) - 1)
#define RING_LEFT(r) (((r)->tail - (r)->head) % RING_SIZE(r))
#define RING_EMPTY(r) (RING_LEFT(r) == 0)
#define RING_FULL(r) (RING_LEFT(r) == RING_CAPACITY(r))

#define RING_PEEK(r) ((r)->buf[(r)->head])
#define RING_GET(r, c)  do { \
	(c) = (r)->buf[(r)->head++]; \
	(r)->head = ((r)->head + 1) % RING_SIZE(r); \
} while (0)
#define RING_PUT(r, c)  do { \
	(r)->buf[(r)->tail++] = (c); \
	(r)->tail = ((r)->tail + 1) % RING_SIZE(r); \
} while (0)

#endif

#ifndef _DS_RING_H
#define _DS_RING_H 1

#include <stddef.h>

#define ring_buffer(type, size) \
	struct { \
		unsigned int head, tail; \
		type buf[size]; \
	}
#define RING_INIT { .head = 0, .tail = 0 }

#define ring_type(r) typeof((r)->buf[0])
#define ring_size(r) array_sizeof((r)->buf)
#define ring_capacity(r) (ring_size(r) - 1)
#define ring_left(r) (((r)->tail - (r)->head) % ring_size(r))
#define ring_empty(r) (ring_left(r) == 0)
#define ring_full(r) (ring_left(r) == ring_capacity(r))

#define ring_peek(r) ((r)->buf[(r)->head])
#define ring_get(r, c)  do { \
	(c) = (r)->buf[(r)->head++]; \
	(r)->head = ((r)->head + 1) % ring_size(r); \
} while (0)
#define ring_put(r, c)  do { \
	(r)->buf[(r)->tail++] = (c); \
	(r)->tail = ((r)->tail + 1) % ring_size(r); \
} while (0)

#endif

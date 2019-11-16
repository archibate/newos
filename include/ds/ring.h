#ifndef _DS_RING_H
#define _DS_RING_H 1

#include <stddef.h>

#define sring_t(type, size) \
	struct { \
		unsigned int head, tail; \
		type buf[size]; \
	}
#define ring_t(type) \
	struct { \
		unsigned int head, tail; \
		type *buf; \
		size_t size; \
	}
#define SRING_INIT { .head = 0, .tail = 0 }
#define RING_INIT(_buf, _size) { .head = 0, .tail = 0, .buf = (_buf), .size = (_size) }

#define ring_init(r, _buf, _size) do { \
	(r)->head = (r)->tail = 0; \
	(r)->buf = (_buf); \
	(r)->size = (_size); \
} while (0)
#define ring_type(r) typeof((r)->buf[0])
#define ring_capacity(r) ((r)->size - 1)
#define ring_left(r) (((r)->tail - (r)->head) % (r)->size)
#define ring_empty(r) (ring_left(r) == 0)
#define ring_full(r) (ring_left(r) == ring_capacity(r))
#define ring_peek(r) ((r)->buf[(r)->head])
#define ring_get(r, c)  do { \
	(c) = (r)->buf[(r)->head++]; \
	(r)->head = ((r)->head + 1) % (r)->size; \
} while (0)
#define ring_put(r, c)  do { \
	(r)->buf[(r)->tail++] = (c); \
	(r)->tail = ((r)->tail + 1) % (r)->size; \
} while (0)

#define sring_init(r) ((void)((r)->head = (r)->tail = 0))
#define sring_type(r) typeof((r)->buf[0])
#define sring_size(r) array_sizeof((r)->buf)
#define sring_capacity(r) (sring_size(r) - 1)
#define sring_left(r) (((r)->tail - (r)->head) % sring_size(r))
#define sring_empty(r) (sring_left(r) == 0)
#define sring_full(r) (sring_left(r) == sring_capacity(r))
#define sring_peek(r) ((r)->buf[(r)->head])
#define sring_get(r, c)  do { \
	(c) = (r)->buf[(r)->head++]; \
	(r)->head = ((r)->head + 1) % sring_size(r); \
} while (0)
#define sring_put(r, c)  do { \
	(r)->buf[(r)->tail++] = (c); \
	(r)->tail = ((r)->tail + 1) % sring_size(r); \
} while (0)

#endif

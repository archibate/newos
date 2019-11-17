#ifndef _BITS_MMAN_H
#define _BITS_MMAN_H 1

#define PROT_EXEC	1
#define PROT_WRITE	2
#define PROT_READ	4
#define PROT_NONE	0

#define MAP_FIXED	1
#define MAP_PRIVATE	0
#define MAP_SHARED	2
#define MAP_ANONYMOUS	4
#define MAP_NOREPLACE	8
#define MAP_FIXED_NOREPLACE	(MAP_FIXED | MAP_NOREPLACE)

#define MS_SYNC		0
#define MS_ASYNC	1
#define MS_INVALIDATE	2

#define MAP_FAILED	((void *)-1)

#endif

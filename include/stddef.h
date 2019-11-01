#ifndef _STDDEF_H
#define _STDDEF_H 1

#include <bits/types.h>

#ifndef size_t
#define size_t __size_t
#endif
#ifndef ptrdiff_t
#define ptrdiff_t __ptrdiff_t
#endif
#ifndef wchar_t
#define wchar_t __wchar_t
#endif
#ifndef wint_t
#define wint_t __wint_t
#endif

#define NULL ((void *)0)
#define offsetof(type, memb) ((size_t)&((type *)NULL)->memb)
#define array_sizeof(a) ((sizeof(a) / sizeof((a)[0])))
#define static_assert(x) switch (0) { case 0: case (x):; }
#define container_of(ptr, type, memb) ((type *)(void *)(ptr) - offsetof(type, memb))

#endif

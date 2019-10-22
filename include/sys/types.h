#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H 1

#include <bits/types.h>

#ifndef size_t
#define size_t __size_t
#endif
#ifndef ssize_t
#define ssize_t __ssize_t
#endif
#ifndef off_t
#define off_t __off_t
#endif
#ifndef clock_t
#define clock_t __clock_t
#endif
#ifndef time_t
#define time_t __time_t
#endif
#ifndef suseconds_t
#define suseconds_t __suseconds_t
#endif
#ifndef pid_t
#define pid_t __pid_t
#endif

#endif

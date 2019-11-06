#ifndef _BITS_TYPES_H
#define _BITS_TYPES_H 1

typedef unsigned char __uint8_t, __uint_fast8_t;
typedef unsigned short __uint16_t;
typedef unsigned int __uint32_t, __uint_least8_t, __uint_least16_t,
	__uint_least32_t, __uint_fast16_t, __uint_fast32_t, __wint_t,
	__dev_t, __ino_t, __nlink_t, __mode_t, __id_t, __uid_t, __gid_t;
typedef unsigned long __uintptr_t, __size_t, __clock_t, __time_t, __suseconds_t;
typedef unsigned long long __uint64_t, __uint_least64_t, __uint_fast64_t,
	__uintmax_t;

typedef char __int8_t, __int_fast8_t;
typedef short __int16_t;
typedef int __int32_t, __int_least8_t, __int_least16_t, __int_least32_t,
	__int_fast16_t, __int_fast32_t, __pid_t, __blksize_t, __blkcnt_t;
typedef long __intptr_t, __ssize_t, __off_t, __ptrdiff_t, __wchar_t;
typedef long long __int64_t, __int_least64_t, __int_fast64_t, __intmax_t;

#endif

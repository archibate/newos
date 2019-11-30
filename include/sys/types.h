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
#ifndef id_t
#define id_t __id_t
#endif
#ifndef pid_t
#define pid_t __pid_t
#endif
#ifndef uid_t
#define uid_t __uid_t
#endif
#ifndef gid_t
#define gid_t __gid_t
#endif
#ifndef ino_t
#define ino_t __ino_t
#endif
#ifndef mode_t
#define mode_t __mode_t
#endif
#ifndef dev_t
#define dev_t __dev_t
#endif
#ifndef nlink_t
#define nlink_t __nlink_t
#endif
#ifndef blksize_t
#define blksize_t __blksize_t
#endif
#ifndef blkcnt_t
#define blkcnt_t __blkcnt_t
#endif
#ifndef key_t
#define key_t __key_t
#endif

#endif

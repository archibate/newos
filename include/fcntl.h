#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <bits/types.h>

#ifndef mode_t
#define mode_t __mode_t
#endif
#ifndef off_t
#define off_t __off_t
#endif
#ifndef pid_t
#define pid_t __pid_t
#endif

#include <bits/fcntl.h>

int creat(const char *path, mode_t mode);
int open(const char *path, int flags, ...);
int openat(int fd, const char *path, int flags, ...);
int fcntl(int fd, int cmd, ...);

#endif

#ifndef _UNISTD_H
#define _UNISTD_H

#include <bits/types.h>

#ifndef size_t
#define size_t __size_t
#endif
#ifndef ssize_t
#define ssize_t __ssize_t
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
#ifndef intptr_t
#define intptr_t __intptr_t
#endif
#ifndef off_t
#define off_t __off_t
#endif
#ifndef pid_t
#define pid_t __pid_t
#endif

#include <bits/types.h>

ssize_t read(int fd, void *buf, size_t size);
ssize_t write(int fd, const void *buf, size_t size);
off_t write(int fd, const void *buf, size_t size);
int dup2(int fd, int fd2);
int dup(int fd);
int pause(void);
pid_t fork(void);

#endif

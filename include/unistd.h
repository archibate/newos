#ifndef _UNISTD_H
#define _UNISTD_H

#include <bits/types.h>
#include <bits/unistd.h>

#ifndef NULL
#define NULL ((void *)0)
#endif
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

ssize_t read(int fd, void *buf, size_t size);
ssize_t write(int fd, const void *buf, size_t size);
off_t write(int fd, const void *buf, size_t size);
off_t lseek(int fd, off_t offset, int whence);
int dup2(int fd, int fd2);
int dup(int fd);
int close(int fd);
int pause(void);
pid_t fork(void);
int execve(const char *path, char *const *argv, char *const *envp);
int execvpe(const char *file, char *const *argv, char *const *envp);
int execv(const char *path, char *const *argv);
int execvp(const char *file, char *const *argv);
int execle(const char *path, const char *arg, ...);
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
__attribute__((noreturn)) void _exit(int status);
void *sbrk(intptr_t incptr);
int brk(void *addr);
int rmdir(const char *path);
int unlink(const char *path);
int link(const char *path1, const char *path2);

#endif

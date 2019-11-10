#ifndef _SYS_STAT_H
#define _SYS_STAT_H 1

#include <sys/types.h>
#include <bits/stat.h>

int mkdir(const char *path, mode_t mode);
int mkdirat(int fd, const char *path, mode_t mode);
int stat(const char *path, struct stat *st);
int lstat(const char *path, struct stat *st);
int fstatat(int fd, const char *path, struct stat *st, int flag);
int fstat(int fd, struct stat *st);

#endif

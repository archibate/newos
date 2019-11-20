#ifndef _ERRNO_H
#define _ERRNO_H 1

#include <bits/errno.h>

#ifndef _LIBC_EXP
#define errno (*IMP$errno)
#endif
extern int errno;

#endif

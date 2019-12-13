#ifndef _ERRNO_H
#define _ERRNO_H 1

#include <bits/errno.h>

extern int *__errno_location(void);
#define errno (*__errno_location())

#endif

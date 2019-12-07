#ifndef _RAX_RAX_H
#define _RAX_RAX_H 1

#include <rax/bits.h>
#include <idl/rax.h>

int XClientInit(void);
int XListen(int hlst, struct Message *msg);

#endif

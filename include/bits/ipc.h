#ifndef _BITS_IPC_H
#define _BITS_IPC_H

#include <sys/types.h>

// for msgget:
#define IPC_CREAT	0x1000
#define IPC_EXCL	0x2000
#define IPC_CLOEXEC	0x4000
// for msgrcv & msgsnd:
#define IPC_NOWAIT	1
// for msgctl:
#define IPC_STAT	1
#define IPC_SET		2
#define IPC_RMID	3

struct ipc_perm
{
	mode_t mode;
	uid_t uid;
	gid_t gid;
	uid_t cuid;
	gid_t cgid;
};

#endif

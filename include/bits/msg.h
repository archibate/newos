#ifndef _BITS_MSG_H
#define _BITS_MSG_H

#include <sys/types.h>
#include <bits/ipc.h>

#define MSG_NOERROR	2

typedef size_t msgqnum_t;
typedef size_t msglen_t;

struct msqid_ds {
	struct ipc_perm msg_perm;
	msgqnum_t msg_qnum;
	msglen_t msg_qbytes;
	pid_t msg_lspid;
	pid_t msg_lrpid;
	time_t msg_stime;
	time_t msg_rtime;
	time_t msg_ctime;
};

#endif

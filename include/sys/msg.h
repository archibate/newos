#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#include <bits/msg.h>

int msgget(key_t key, int msgflg);
int msgctl(int msqid, int cmd, struct msqid_ds *buf);
ssize_t msgrcv(int msqid, void *msgb, size_t msgsz, long msgtyp, int msgflg);
int msgsnd(int msqid, const void *msgb, size_t msgsz, int msgflg);

#endif

#ifndef _SYS_LEPTON_H
#define _SYS_LEPTON_H

#include <bits/lepton.h>

int Connect(int fd, pid_t pid, id_t chid);
int CreateChannel(id_t chid);
int MsgReceive(id_t chid, struct msgio *b, size_t iovnr);
int MsgSend(int fd, struct msgio *b, size_t iovnr);

#endif

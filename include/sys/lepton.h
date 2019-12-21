#ifndef _SYS_LEPTON_H
#define _SYS_LEPTON_H

#include <bits/lepton.h>

int Connect(int fd, pid_t pid, id_t chid);
int CreateChannel(id_t chid);
int MsgSend(int fd, struct msgio *b, size_t iovnr,
		struct msgio *r_b, size_t r_iovnr);
int MsgReceive(id_t chid, struct msgio *b, size_t iovnr);
int MsgReply(id_t chid, struct msgio *b, size_t iovnr);

#endif

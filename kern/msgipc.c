#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/fs.h>
#include <string.h>
#include <bits/msg.h>
#include <ds/list.h>
#include <malloc.h>
#include <errno.h>

struct msqueue
{
	struct list_node list;

	struct task *wait_send;
	struct task *wait_recv;

	int type;
	char *buf;
	size_t size;

	struct task *sender_now;
	struct task *recver_now;

	key_t key;
	struct msqid_ds msq;
};

static struct list_head msqs;

static struct msqueue *create_msqueue(key_t key, mode_t mode)
{
	struct msqueue *msq = calloc(sizeof(struct msqueue), 1);
	msq->msq.msg_perm.mode = mode;
	msq->msq.msg_qnum = -1;
	msq->msq.msg_qbytes = 1024;
	msq->buf = malloc(msq->msq.msg_qbytes);
	msq->key = key;
	list_insert_head(&msq->list, &msqs);
	return msq;
}

void destroy_msqueue(struct msqueue *msq)
{
	// TODO: post EIDRM
	list_remove(&msq->list);
	free(msq);
}

ssize_t msqueue_receive(struct msqueue *msq, int *type,
		void *buf, size_t size)
{
	if (size > msq->msq.msg_qbytes) {
		errno = EINVAL;
		return -1;
	}
	while (!msq->sender_now ||
		(msq->recver_now && msq->recver_now != current)) {
		msq->recver_now = current;
		sleep_on(&msq->wait_recv);
	}
	msq->recver_now = current;
	if (size < msq->size) {
		size = msq->size;
	}
	*type = msq->type;
	size = msq->size;
	memcpy(buf, msq->buf, size);
	msq->sender_now = 0;
	wake_up(&msq->wait_send);
	return size;
}

int msqueue_send(struct msqueue *msq, int type,
		const void *buf, size_t size)
{
	if (size > msq->msq.msg_qbytes) {
		errno = EINVAL;
		return -1;
	}
	while (!msq->recver_now ||
		(msq->sender_now && msq->sender_now != current)) {
		msq->sender_now = current;
		sleep_on(&msq->wait_send);
	}
	msq->sender_now = current;
	msq->type = type;
	msq->size = size;
	memcpy(msq->buf, buf, size);
	msq->recver_now = 0;
	wake_up(&msq->wait_recv);
	return 0;
}

int sys_msgget(key_t key, int msgflg)
{
	struct msqueue *msq;
	list_foreach(msq, &msqs, list) {
		if (msq->key == key) {
			if (msgflg & IPC_EXCL) {
				errno = EEXIST;
				return -1;
			}
			goto found;
		}
	}
	if (!(msgflg & IPC_CREAT)) {
		errno = ENOENT;
		return -1;
	}
	msq = create_msqueue(key, msgflg & 0777);
found:
	msgflg = msgflg & IPC_CLOEXEC ? O_CLOEXEC : 0;
	int ret = open_fd_object(msq, FT_MSGQ, msgflg);
	if (ret == -1)
		destroy_msqueue(msq);
	return ret;
}

int sys_msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
	struct msqueue *msq = get_fd_object(msqid, FT_MSGQ);
	if (!msq) {
		errno = EINVAL;
		return -1;
	}
	switch (cmd) {
	case IPC_RMID:
		destroy_msqueue(msq);
		extern int sys_close(int fd);
		sys_close(msqid);
		return 0;
	case IPC_STAT:
		memcpy(buf, &msq->msq, sizeof(struct msqid_ds));
		return 0;
	case IPC_SET:
		memcpy(&msq->msq, buf, sizeof(struct msqid_ds));
		return 0;
	}
	errno = EINVAL;
	return -1;
}

ssize_t sys_msgrcv(int msqid, void *msgb, size_t msgsz,
		long msgtyp, int msgflg)
{
	struct msqueue *msq = get_fd_object(msqid, FT_MSGQ);
	if (!msq) {
		errno = EINVAL;
		return -1;
	}
	return msqueue_receive(msq, (int *)msgb, msgb + sizeof(long), msgsz);
}

int sys_msgsnd(int msqid, const void *msgb, size_t msgsz, int msgflg)
{
	struct msqueue *msq = get_fd_object(msqid, FT_MSGQ);
	if (!msq) {
		errno = EINVAL;
		return -1;
	}
	return msqueue_send(msq, *(int *)msgb, msgb + sizeof(long), msgsz);
}
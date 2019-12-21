#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/lepton.h>

#if 0
ssize_t _read(int fd, void *buf, size_t size)
{
	struct hdr {
		int fd;
	} hdr = {fd};
	struct msgio b[2] = {
		&hdr, sizeof(hdr),
		buf, size,
	};
	MsgSend(fd, b, 2);
}
#endif

#define PARENT_CHID 1

pid_t parent_pid;

char buf[3][233];
char rep[1][233];
struct msgio b[3] = {
	{buf[0], 233},
	{buf[1], 233},
	{buf[2], 233},
};
struct msgio r[1] = {
	{rep[0], 233},
};

static void child(void)
{
	int fd = 5;
	if (-1 == Connect(fd, parent_pid, PARENT_CHID)) {
		perror("child: Connect");
		return;
	}
	printf("child: connected to %d/%d\n", parent_pid, PARENT_CHID);
	strcpy(b[0].buf, "Hello, ");
	strcpy(b[1].buf, "par");
	strcpy(b[2].buf, "ent!");
	b[0].size = strlen(b[0].buf);
	b[1].size = strlen(b[1].buf);
	b[2].size = strlen(b[2].buf);
	if (-1 == MsgSend(fd, b, 3, r, 1)) {
		perror("child: MsgSend");
		return;
	}
	printf("child: got reply [%.*s]\n", (int)r->size, r->buf);
}

static void parent(void)
{
	int chid = PARENT_CHID;
	if (-1 == CreateChannel(chid)) {
		perror("parent: CreateChannel");
		return;
	}
	printf("parent: listening on %d/%d\n", parent_pid, chid);
	if (-1 == MsgReceive(chid, b, 1)) {
		perror("parent: MsgReceive");
		return;
	}
	printf("parent: received data [%.*s]\n", (int)b->size, b->buf);
	strcpy(b[0].buf, "Hello, C");
	strcpy(b[1].buf, "hild!");
	b[0].size = strlen(b[0].buf);
	b[1].size = strlen(b[1].buf);
	if (-1 == MsgReply(chid, b, 2)) {
		perror("parent: MsgReply");
		return;
	}
}

int main(int argc, char **argv)
{
	parent_pid = getpid();
	pid_t pid = fork();
	if (pid) {
		parent();
		waitpid(pid, NULL, 0);
	} else if (pid == 0) {
		child();
		exit(0);
	} else if (pid < 0) {
		perror("fork");
	}
	return 0;
}

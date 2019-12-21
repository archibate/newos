#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/lepton.h>

#define PARENT_CHID 1

pid_t parent_pid;

char buf[3][233];
struct msgio b[3] = {
	{buf[0], 233},
	{buf[1], 233},
	{buf[2], 233},
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
	if (-1 == MsgSend(fd, b, 3)) {
		perror("child: MsgSend");
		return;
	}
	printf("child: data sent\n");
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

#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct msg {
	int type;
	char text[10];
};

static void child(void)
{
	key_t key = ftok(".", 233);
	if (key == -1) {
		perror("child: ftok");
		return;
	}
	printf("child: using key %#lx...\n", key);
	int msqid = msgget(key, 0);
	if (msqid == -1) {
		perror("child: msgget");
		return;
	}
	struct msg msg = {233, "Hello, IPC"};
	printf("child sending type=%d text=[%.10s]\n", msg.type, msg.text);
	int ret = msgsnd(msqid, &msg, sizeof(msg.text), 0);
	printf("child message sent\n");
	return;
}

static void parent(void)
{
	key_t key = ftok(".", 233);
	if (key == -1) {
		perror("parent: ftok");
		return;
	}
	printf("parent: using key %#lx...\n", key);
	int msqid = msgget(key, IPC_CREAT | IPC_EXCL | 0644);
	if (msqid == -1) {
		perror("parent: msgget");
		return;
	}
	struct msg msg;
	printf("parent waiting for message...\n");
	int ret = msgrcv(msqid, &msg, sizeof(msg.text), 0, 0);
	printf("parent received type=%d text=[%.10s]\n", msg.type, msg.text);
	return;
}

int main(int argc, char **argv)
{
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

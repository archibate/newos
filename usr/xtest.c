#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>

static int g_msq;

static int XOpenServer(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1)
		return -1;
	g_msq = msgget(key, 0);
	if (g_msq == -1)
		return -1;
	return 0;
}

static int XSplashScreen(int num)
{
	struct msg {
		long cmd;
		long num;
	} msg;
	msg.cmd = 1; // __X_SplashScreen
	msg.num = num;
	if (-1 == msgsnd(g_msq, &msg, sizeof(msg.num), 0))
		return -1;
	return 0;
}

int main(int argc, char **argv)
{
	for (volatile int j = 0; j < 1e7; j++);
	if (XOpenServer() == -1) {
		perror("cannot open X server");
		return 1;
	}

	for (int i = 2; i <= 320 * 5; i++) {
		XSplashScreen(i);
	}
	return 0;
}

#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int g_nx, g_ny, g_msq;
static char *g_buf;
static FILE *g_f;

static void UpdateScreen(void)
{
	rewind(g_f);
	if (fwrite(g_buf, g_nx * g_ny, 1, g_f) != 1)
		perror("cannot update video buffer");
}

static void do_XSplashScreen(int num)
{
	rewind(g_f);
	for (int i = 0; i < g_nx * g_ny; i++)
		g_buf[i] = i % num;
	UpdateScreen();
}

static void do_X_command(long cmd, const long *args)
{
	switch (cmd) {
	case 1: do_XSplashScreen(args[0]); break;
	default: printf("bad X command %ld (%#lx)\n", cmd, cmd);
	}
}

int main(int argc, char **argv)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("cannot fork to background");
		return 1;
	}
	if (pid > 0) {
		printf("X server forked to background pid=%d\n", pid);
		return 0;
	}
	stdout = stderr;

	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1) {
		perror("/dev/fb0");
		return 1;
	}

	g_msq = msgget(key, IPC_CREAT | IPC_EXCL);
	if (g_msq == -1) {
		perror("cannot create msqueue for X server");
		return 1;
	}

	g_f = fopen("/dev/fb0", "w+");
	if (!g_f) {
		perror("/dev/fb0");
		return 1;
	}
	g_nx = ioctl(fileno(g_f), I_FB_GET_NX);
	g_ny = ioctl(fileno(g_f), I_FB_GET_NY);
	if (g_nx == -1 || g_ny == -1) {
		perror("cannot ioctl /dev/fb0");
		return 1;
	}

	g_buf = malloc(g_nx * g_ny);

	struct msg {
		long cmd;
		long args[20];
	} msg;

	ssize_t size;
	while (1) {
		size = msgrcv(g_msq, &msg, sizeof(msg.args), 0, 0);
		if (-1 == size)
			perror("cannot receive from X client");
		do_X_command(msg.cmd, msg.args);
	}

	fclose(g_f);
	return 0;
}

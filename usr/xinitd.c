#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int g_nx, g_ny, g_msq;
static char *g_buf;
static FILE *g_f;
static int l_alpha[200];
static int l_color[200];

#define L(x, y) ((x) + g_nx * (y))

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

static int do_XCreateDC(int flags)
{
	static int hdc = 1;
	return hdc++;
}

static void do_XDestroyDC(int hdc)
{
}

static void do_XSetFillStyle(int hdc, int color, int alpha)
{
	l_color[hdc] = color & 0xff;
	l_alpha[hdc] = alpha & 0xff;
}

static void do_XFillRect(int hdc, int x0, int y0, int x1, int y1)
{
	int nx = x1 - x0;
	int ny = y1 - y0;
	if (nx < 0) {
		x0 += nx;
		nx = -nx;
	}
	if (ny < 0) {
		y0 += ny;
		ny = -ny;
	}
	int col = l_color[hdc];
	int alpha = l_alpha[hdc];
	if (!alpha) {
		for (int y = y0; y < y0 + ny; y++)
			for (int i = L(x0, y); i < L(x0 + nx, y); i++)
				g_buf[i] = col;
	} else if (alpha) {
		col *= 256 - alpha;
		for (int y = y0; y < y0 + ny; y++)
			for (int i = L(x0, y); i < L(x0 + nx, y); i++)
				g_buf[i] = (g_buf[i] * alpha + col) >> 8;
	}
	UpdateScreen();
}

static void do_X_command(long cmd, const long *a)
{
	struct msg {
		long cmd;
		long ret;
	} rep;
	switch (cmd & 0xffff) {
	case 1:
		do_XSplashScreen(a[0]);
		break;
	case 2:
		do_XFillRect(a[0], a[1], a[2], a[3], a[4]);
		break;
	case 3:
		do_XSetFillStyle(a[0], a[1], a[2]);
		break;
	case 4:
		rep.cmd = cmd;
		rep.ret = do_XCreateDC(a[0]);
		printf("XCreateDC(%ld) = %ld\n", a[0], rep.ret);
		msgsnd(g_msq, &rep, sizeof(rep.ret),
				MSG_NOERROR | IPC_NOWAIT);
		break;
	case 5:
		do_XDestroyDC(a[0]);
		break;
	default: printf("bad X command %ld (%#lx)\n", cmd, cmd);
	}
}

int main(int argc, char **argv)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("cannot fork X server to background");
		return 1;
	}
	if (pid > 0) {
		printf("X-server started in background, pid=%d\n", pid);
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
		memset(&msg, 0, sizeof(msg));
		size = msgrcv(g_msq, &msg, sizeof(msg.args), 0, MSG_NOERROR);
		if (-1 == size) {
			perror("cannot receive from X client");
			break;
		}
		do_X_command(msg.cmd, msg.args);
	}

	fclose(g_f);
	return 0;
}

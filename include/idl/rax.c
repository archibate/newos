void XSplashScreen(int num)
{
	struct msg {
		long cmd;
		int num;
	} __attribute__((packed)) msg;
	msg.cmd = 1;
	msg.num = num;
	msgsnd(g_msq, &msg, sizeof(msg) - sizeof(msg.cmd), 0);
}

void XFillRect(int hdc, int x0, int y0, int x1, int y1)
{
	struct msg {
		long cmd;
		int hdc;
		int x0;
		int y0;
		int x1;
		int y1;
	} __attribute__((packed)) msg;
	msg.cmd = 2;
	msg.hdc = hdc;
	msg.x0 = x0;
	msg.y0 = y0;
	msg.x1 = x1;
	msg.y1 = y1;
	msgsnd(g_msq, &msg, sizeof(msg) - sizeof(msg.cmd), 0);
}

void XSetFillStyle(int hdc, int color, int alpha)
{
	struct msg {
		long cmd;
		int hdc;
		int color;
		int alpha;
	} __attribute__((packed)) msg;
	msg.cmd = 3;
	msg.hdc = hdc;
	msg.color = color;
	msg.alpha = alpha;
	msgsnd(g_msq, &msg, sizeof(msg) - sizeof(msg.cmd), 0);
}

void XCreateDC(int *hdc, int flags)
{
	struct msg {
		long cmd;
		int flags;
	} __attribute__((packed)) msg;
	msg.cmd = 4;
	msg.flags = flags;
	msgsnd(g_msq, &msg, sizeof(msg) - sizeof(msg.cmd), 0);
	struct rep {
		long cmd;
		int hdc;
	} __attribute__((packed)) rep;
	msgrcv(g_msq, &rep, sizeof(rep) - sizeof(rep.cmd), msg.cmd, 0);
	*hdc = rep.hdc;
}

void XDestroyDC(int hdc)
{
	struct msg {
		long cmd;
		int hdc;
	} __attribute__((packed)) msg;
	msg.cmd = 5;
	msg.hdc = hdc;
	msgsnd(g_msq, &msg, sizeof(msg) - sizeof(msg.cmd), 0);
}


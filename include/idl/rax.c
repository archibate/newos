void XGetDC(int *hdc, int hwnd)
{
	struct msg {
		long cmd;
		int hwnd;
	} m;
	m.cmd = 1;
	m.hwnd = hwnd;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
	struct rep {
		long seq;
		int hdc;
	} r;
	msgrcv(g_msq_r, &r, sizeof(r) - sizeof(r.seq), 0, MSG_REPLYSEQ);
	*hdc = r.hdc;
}

void XDestroyDC(int hint)
{
	struct msg {
		long cmd;
		int hint;
	} m;
	m.cmd = 2;
	m.hint = hint;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XSetFillStyle(int hdc, int color)
{
	struct msg {
		long cmd;
		int hdc;
		int color;
	} m;
	m.cmd = 3;
	m.hdc = hdc;
	m.color = color;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
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
	} m;
	m.cmd = 4;
	m.hdc = hdc;
	m.x0 = x0;
	m.y0 = y0;
	m.x1 = x1;
	m.y1 = y1;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XCreateWindow(int *hwnd, int hparent, int x0, int y0, int nx, int ny, int flags)
{
	struct msg {
		long cmd;
		int hparent;
		int x0;
		int y0;
		int nx;
		int ny;
		int flags;
	} m;
	m.cmd = 5;
	m.hparent = hparent;
	m.x0 = x0;
	m.y0 = y0;
	m.nx = nx;
	m.ny = ny;
	m.flags = flags;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
	struct rep {
		long seq;
		int hwnd;
	} r;
	msgrcv(g_msq_r, &r, sizeof(r) - sizeof(r.seq), 0, MSG_REPLYSEQ);
	*hwnd = r.hwnd;
}

void XUpdateWindow(int hwnd)
{
	struct msg {
		long cmd;
		int hwnd;
	} m;
	m.cmd = 6;
	m.hwnd = hwnd;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}


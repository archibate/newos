#include <stdio.h>

void XCreateDC(int *hdc, int hwnd)
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

void XUpdateDC(int hdc)
{
	struct msg {
		long cmd;
		int hdc;
	} m;
	m.cmd = 2;
	m.hdc = hdc;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XDestroyDC(int hdc)
{
	struct msg {
		long cmd;
		int hdc;
	} m;
	m.cmd = 3;
	m.hdc = hdc;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XSetFillStyle(int hdc, int bgcolor, int fgcolor)
{
	struct msg {
		long cmd;
		int hdc;
		int bgcolor;
		int fgcolor;
	} m;
	m.cmd = 4;
	m.hdc = hdc;
	m.bgcolor = bgcolor;
	m.fgcolor = fgcolor;
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
	m.cmd = 5;
	m.hdc = hdc;
	m.x0 = x0;
	m.y0 = y0;
	m.x1 = x1;
	m.y1 = y1;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XTextOut(int hdc, int x0, int y0, const char *text, int count)
{
	struct msg {
		long cmd;
		int hdc;
		int x0;
		int y0;
		char text[128];
		int count;
	} m;
	m.cmd = 6;
	m.hdc = hdc;
	m.x0 = x0;
	m.y0 = y0;
	memcpy(m.text, text, 128 * sizeof(char));
	m.count = count;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XSetPixel(int hdc, int x0, int y0)
{
	struct msg {
		long cmd;
		int hdc;
		int x0;
		int y0;
	} m;
	m.cmd = 7;
	m.hdc = hdc;
	m.x0 = x0;
	m.y0 = y0;
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
	m.cmd = 8;
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
	m.cmd = 9;
	m.hwnd = hwnd;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XDestroyWindow(int hwnd)
{
	struct msg {
		long cmd;
		int hwnd;
	} m;
	m.cmd = 10;
	m.hwnd = hwnd;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XRefreshWindow(int hwnd, int deep)
{
	struct msg {
		long cmd;
		int hwnd;
		int deep;
	} m;
	m.cmd = 11;
	m.hwnd = hwnd;
	m.deep = deep;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XSetWindowText(int hwnd, const char *text)
{
	struct msg {
		long cmd;
		int hwnd;
		char text[33];
	} m;
	m.cmd = 12;
	m.hwnd = hwnd;
	memcpy(m.text, text, 33 * sizeof(char));
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XSetWindowPos(int hwnd, int x0, int y0)
{
	struct msg {
		long cmd;
		int hwnd;
		int x0;
		int y0;
	} m;
	m.cmd = 13;
	m.hwnd = hwnd;
	m.x0 = x0;
	m.y0 = y0;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XCreateListener(int *hlst)
{
	struct msg {
		long cmd;
	} m;
	m.cmd = 14;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
	struct rep {
		long seq;
		int hlst;
	} r;
	msgrcv(g_msq_r, &r, sizeof(r) - sizeof(r.seq), 0, MSG_REPLYSEQ);
	*hlst = r.hlst;
}

void XDestroyListener(int hlst)
{
	struct msg {
		long cmd;
		int hlst;
	} m;
	m.cmd = 15;
	m.hlst = hlst;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}

void XListenerBind(int hlst, int hwnd, int deep)
{
	struct msg {
		long cmd;
		int hlst;
		int hwnd;
		int deep;
	} m;
	m.cmd = 16;
	m.hlst = hlst;
	m.hwnd = hwnd;
	m.deep = deep;
	msgsnd(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0);
}


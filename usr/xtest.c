#ifdef _VIDEO
#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <rax/rax.h>

static int hdc, hwnd, hbtn, hchld, hlbl, hlst;

static void do_message(struct Message *msg)
{
	switch (msg->type) {
	case WM_MOUSE_LDOWN:
		if (msg->hwnd == hbtn) {
			XSetWindowText(hlbl, "Mouse Down");
			XUpdateWindow(hlbl);
		}
		break;
	case WM_MOUSE_LUP:
		printf("hwnd %d clicked at (%d, %d)\n",
			msg->hwnd, msg->pos.x, msg->pos.y);
		if (msg->hwnd == hbtn) {
			XSetWindowText(hlbl, "Mouse Up");
			XUpdateWindow(hlbl);
		}
		break;
	}
}

int main(int argc, char **argv)
{
	struct Message msg;
	int i, x0, x1, y0, y1, color;

	if (XClientInit() == -1) {
		perror("cannot connect X server");
		return 1;
	}

	XCreateWindow(&hwnd, 0, 30, 30, 200, 150, WT_CAPTION | WF_MOVE);
	XSetWindowText(hwnd, "Window 1");

	XCreateDC(&hdc, hwnd);
	for (i = 0; i <= 1000; i++) {
		x0 = rand() % 200;
		x1 = rand() % 200;
		y0 = rand() % 150;
		y1 = rand() % 150;
		color = rand() % 256;
		XSetFillStyle(hdc, color, 0);
		XFillRect(hdc, x0, y0, x1, y1);
	}
	XUpdateDC(hdc);

	XCreateWindow(&hchld, hwnd, 50, 40, 100, 80, WT_CAPTION | WF_MOVE | WF_CLICK);
	XSetWindowText(hchld, "Window 2");
	XUpdateWindow(hchld);

	XCreateWindow(&hlbl, hchld, 8, 8, 84, 18, WT_LABEL | WF_NOSEL);
	XSetWindowText(hlbl, "Label:");
	XUpdateWindow(hlbl);

	XCreateWindow(&hbtn, hchld, 20, 56, 60, 18, WT_BUTTON | WF_CLICK);
	XSetWindowText(hbtn, "OK");
	XUpdateWindow(hbtn);

	XUpdateWindow(hwnd);

	XCreateListener(&hlst);
	XListenerBind(hlst, hwnd, 1);

	while (1) {
		if (-1 == XListen(hlst, &msg)) {
			perror("XListen");
			continue;
		}
		do_message(&msg);
	}

	XDestroyDC(hdc);
	XDestroyListener(hlst);

	return 0;
}
#endif

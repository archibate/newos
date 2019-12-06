#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <rax/rax.h>

void ff(int aa) {}
int main(int argc, char **argv)
{
	int hdc, hwnd, hbtn;
	if (XOpenServer() == -1) {
		perror("cannot open X server");
		return 1;
	}

	XCreateWindow(&hwnd, 0, 30, 30, 200, 150, WS_CAPTION | WF_MOVE);

	XGetDC(&hdc, hwnd);
	for (int i = 0; i <= 1000; i++) {
		int x0 = rand() % 200;
		int x1 = rand() % 200;
		int y0 = rand() % 150;
		int y1 = rand() % 150;
		int color = rand() % 256;
		XSetFillStyle(hdc, color);
		XFillRect(hdc, x0, y0, x1, y1);
	}
	XDestroyDC(hdc);

	XCreateWindow(&hbtn, hwnd, 70, 120, 60, 18, WS_BUTTON | WF_MOVE);

	XUpdateWindow(hbtn);
	XUpdateWindow(hwnd);
	return 0;
}

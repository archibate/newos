#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <rax/rax.h>

void ff(int aa) {}
int main(int argc, char **argv)
{
	if (XOpenServer() == -1) {
		perror("cannot open X server");
		return 1;
	}

	XSplashScreen(16);

	int hdc;
	XCreateDC(&hdc, 0);
	int alpha = argv[1] ? atoi(argv[1]) : 0;
	for (int i = 0; i <= 320; i++) {
		int x0 = rand() % 320;
		int x1 = rand() % 320;
		int y0 = rand() % 200;
		int y1 = rand() % 200;
		int color = rand() % 256;
		XSetFillStyle(hdc, color, alpha);
		XFillRect(hdc, x0, y0, x1, y1);
	}
	XDestroyDC(hdc);
	return 0;
}

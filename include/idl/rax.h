void XGetDC(int *hdc, int hwnd);
void XDestroyDC(int hint);
void XSetFillStyle(int hdc, int color);
void XFillRect(int hdc, int x0, int y0, int x1, int y1);
void XCreateWindow(int *hwnd, int hparent, int x0, int y0, int nx, int ny, int flags);
void XUpdateWindow(int hwnd);

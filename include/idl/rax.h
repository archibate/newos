void XCreateDC(int *hdc, int hwnd);
void XUpdateDC(int hdc);
void XDestroyDC(int hdc);
void XSetFillStyle(int hdc, int color);
void XFillRect(int hdc, int x0, int y0, int x1, int y1);
void XCreateWindow(int *hwnd, int hparent, int x0, int y0, int nx, int ny, int flags);
void XUpdateWindow(int hwnd);
void XDestroyWindow(int hwnd);
void XRefreshWindow(int hwnd, int deep);
void XSetWindowText(int hwnd, const char *text);
void XSetWindowPos(int hwnd, int x0, int y0);
void XCreateListener(int *hlst);
void XDestroyListener(int hlst);
void XListenerBind(int hlst, int hwnd, int deep);
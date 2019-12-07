#ifdef _VIDEO
#include "busybox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <sys/notify.h>
#include <signal.h>
#include <rax/bits.h>
#include <unistd.h>
#include <fcntl.h>

// Part of Handle: {{{
enum handle_type {
	HT_None = 0,
	HT_DC, HT_Window, HT_Listener,
};

struct handle {
	struct handle *next, **pprev;
	int hint;
	int type;
};

#define HASH_MAX 23
#define HASH(x) ((x) * 257 % 23)

static struct handle *handles[HASH_MAX];

static void *GetHandle(int hint, int type)
{
	int i = HASH(hint);
	struct handle *h;
	for (h = handles[i]; h; h = h->next) {
		if (h->hint == hint) {
			if (!type || h->type == type)
				return h;
			else
				break;
		}
	}
	return NULL;
}

static int now_hint_top = 1;

static void *CreateHandle(int type, size_t size)
{
	int hint = now_hint_top++;
	int i = HASH(hint);
	struct handle *h = calloc(size, 1);
	h->hint = hint;
	h->type = type;
	h->next = handles[i];
	if (handles[i])
		handles[i]->pprev = &h->next;
	h->pprev = &handles[i];
	handles[i] = h;
	return h;
}

static void DestroyHandle(void *hp)
{
	struct handle *h = hp;
	if (h->next)
		h->next->pprev = h->pprev;
	*h->pprev = h->next;
	h->pprev = (void *)-1;
	h->next = (void *)-1;
	free(h);
}

// }}}
// Part of Buffer: {{{

#define RG /2+
#define GB *4+32*
typedef unsigned char RGB;

struct buf {
	RGB *rgb;
	int sx, sy;
	int nx, ny;
	int ismybuf;
	int col_inv;
};

static void buf_create(struct buf *b, int nx, int ny)
{
	b->rgb = calloc(nx, ny);
	b->sx = 1;
	b->sy = nx;
	b->nx = nx;
	b->ny = ny;
	b->ismybuf = 1;
	b->col_inv = -1;
}

static void buf_destroy(struct buf *b)
{
	if (b->ismybuf)
		free(b->rgb);
}

static void buf_pointSanity(struct buf *b, int *x, int *y)
{
	if (*x < 0)
		*x = 0;
	if (*y < 0)
		*y = 0;
	if (*x > b->nx)
		*x = b->nx;
	if (*y > b->ny)
		*y = b->ny;
}

static void buf_rectSanity(struct buf *b,
		int *x0, int *y0, int *x1, int *y1)
{
	int nx = *x1 - *x0;
	int ny = *y1 - *y0;
	if (nx < 0) {
		*x0 += nx;
		*x1 -= nx;
	}
	if (ny < 0) {
		*y0 += ny;
		*y1 -= ny;
	}
	buf_pointSanity(b, x0, y0);
	buf_pointSanity(b, x1, y1);
}

static void buf_subRect(struct buf *b, struct buf *d,
		int x0, int y0, int x1, int y1)
{
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	d->rgb = b->rgb + x0 * b->sx + y0 * b->sy;
	d->sx = b->sx;
	d->sy = b->sy;
	d->nx = x1 - x0;
	d->ny = y1 - y0;
	d->ismybuf = 0;
}

static void buf_blitSubSub(struct buf *b, struct buf *d,
		int x0, int y0, int sx0, int sy0, int sx1, int sy1)
{
	RGB *p, *q;
	int x, y, xo = x0 - sx0, yo = y0 - sy0;
	int x1 = x0 + sx1 - sx0, y1 = y0 + sy1 - sy0;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	//printf("blit %p <- %p: %d %d %d %d\n", b, d, x0, y0, x1, y1);
	if (d->col_inv == -1) {
		for (y = y0; y < y1; y++) {
			for (x = x0; x < x1; x++) {
				p = b->rgb + x * b->sx + y * b->sy;
				q = d->rgb + (x - xo) * d->sx + (y - yo) * d->sy;
				*p = *q;
			}
		}
	} else {
		for (y = y0; y < y1; y++) {
			for (x = x0; x < x1; x++) {
				p = b->rgb + x * b->sx + y * b->sy;
				q = d->rgb + (x - xo) * d->sx + (y - yo) * d->sy;
				if (*q != d->col_inv)
					*p = *q;
			}
		}
	}
}
static void buf_blitSub(struct buf *b, struct buf *d,
		int x0, int y0)
{
	buf_blitSubSub(b, d, x0, y0, 0, 0, d->nx, d->ny);
}

static void buf_fillRect(struct buf *b,
		int x0, int y0, int x1, int y1,
		RGB color)
{
	RGB *p;
	int x, y;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			p = b->rgb + x * b->sx + y * b->sy;
			*p = color;
		}
	}
}

static void buf_setPixel(struct buf *b, int x, int y, RGB color)
{
	buf_fillRect(b, x, y, x + 1, y + 1, color);
}

static void buf_textOut(struct buf *b,
		int x0, int y0, const char *s, int count,
		RGB color)
{
	int x1 = x0 + count * 8, y1 = y0 + 16;
	buf_rectSanity(b, &x0, &y0, &x1, &y1);
	extern const char asc16[256 * 16];
	for (int x = x0; x < x1; x += 8) {
		const char *a = asc16 + *s++ * 16;
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 8; j++) {
				if (a[i] & (1 << (7 - j)))
					buf_setPixel(b, x + j, y0 + i, color);
			}
		}
	}
}

// }}}
// Part of Screen: {{{

static int g_nx, g_ny;
static FILE *g_screen_f;
static RGB *g_vram;

static void screen_init(void)
{
	g_screen_f = fopen("/dev/fb0", "w+");
	if (!g_screen_f) {
		perror("/dev/fb0");
		exit(1);
	}
	g_nx = ioctl(fileno(g_screen_f), I_FB_GET_NX);
	g_ny = ioctl(fileno(g_screen_f), I_FB_GET_NY);
	if (g_nx == -1 || g_ny == -1) {
		perror("cannot ioctl /dev/fb0");
		exit(1);
	}
}

static void UpdateScreen(void)
{
	rewind(g_screen_f);
	if (fwrite(g_vram, g_nx * g_ny * sizeof(RGB),
				1, g_screen_f) != 1)
		perror("cannot update video buffer");
}

// }}}
// Part of Listener: {{{

static int g_msq_m;

struct Listener
{
	struct handle h;
};

static void ListenerCallback(struct Listener *l, struct Message *msg)
{
	msg->hlst = l->h.hint;
	msgsnd(g_msq_m, msg, sizeof(*msg) - sizeof(msg->hwnd),
			IPC_NOWAIT | MSG_NOERROR);
}

static void do_XCreateListener(int *hlst)
{
	struct Listener *l = CreateHandle(HT_Listener, sizeof(struct Listener));
	*hlst = l->h.hint;
}

static void do_XDestroyListener(int hlst)
{
	struct Listener *l = GetHandle(hlst, HT_Listener);
	if (!l) return;
	DestroyHandle(l);
}

// }}}
// Part of Window: {{{

#define WMTOP 18

struct Window {
	struct handle h;
	struct buf b, *dcb;
	int x0, y0, zindex;
	int cx0, cy0, cx1, cy1;
	struct Window *parent, *children;
	struct Window *next, **pprev;
	struct Listener *listener;
	int flags;
	int isdown;
	char *text;
};

struct Window *g_desktop;

static void ListenerBind(struct Listener *l, struct Window *w, int deep)
{
	w->listener = l;
	if (deep) {
		struct Window *u;
		for (u = w->children; u; u = u->next)
			ListenerBind(l, u, deep);
	}
}

static void draw_label(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, 4 RG 4 GB 4);
	if (w->text)
		buf_textOut(b, 0,//b->nx / 2 - 4 * strlen(w->text),
				b->ny / 2 - 8, w->text,
				strlen(w->text), 0 RG 0 GB 0);
}

static void draw_button_up(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, 6 RG 6 GB 6);
	buf_fillRect(b, 0, b->ny - 2, b->nx, b->ny, 4 RG 4 GB 4);
	buf_fillRect(b, b->nx - 2, 0, b->nx, b->ny, 4 RG 4 GB 4);
	buf_fillRect(b, 0, 0, b->nx, 1, 7 RG 7 GB 7);
	buf_fillRect(b, 0, 0, 1, b->ny, 7 RG 7 GB 7);
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, 0 RG 0 GB 0);
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, 0 RG 0 GB 0);
	if (w->text)
		buf_textOut(b, b->nx / 2 - 4 * strlen(w->text),
				b->ny / 2 - 8, w->text,
				strlen(w->text), 0 RG 0 GB 0);
}

static void draw_button_down(struct Window *w)
{
	struct buf *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, 4 RG 4 GB 4);
	buf_fillRect(b, 0, 0, b->nx, 2, 2 RG 2 GB 2);
	buf_fillRect(b, 0, 0, 2, b->ny, 2 RG 2 GB 2);
	buf_fillRect(b, 0, 0, b->nx, 1, 0 RG 0 GB 0);
	buf_fillRect(b, 0, 0, 1, b->ny, 0 RG 0 GB 0);
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, 6 RG 6 GB 6);
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, 6 RG 6 GB 6);
	if (w->text)
		buf_textOut(b, b->nx / 2 - 4 * strlen(w->text) + 1,
				b->ny / 2 - 7, w->text,
				strlen(w->text), 0 RG 0 GB 0);
}

static void draw_caption(struct Window *w)
{
	int i;
	struct buf b0, *b = &w->b;
	buf_fillRect(b, 0, 0, b->nx, b->ny, 4 RG 4 GB 4);
	buf_fillRect(b, b->nx - 1, 0, b->nx, b->ny, 2 RG 2 GB 2);
	buf_fillRect(b, 0, b->ny - 1, b->nx, b->ny, 2 RG 2 GB 2);
	buf_fillRect(b, 0, 0, b->nx, 1, 6 RG 6 GB 6);
	buf_fillRect(b, 0, 0, 1, b->ny, 6 RG 6 GB 6);
	buf_fillRect(b, 1, 1, b->nx - 1, WMTOP - 1, 0 RG 0 GB 5);
	if (w->text)
		buf_textOut(b, 4, 2, w->text,
				strlen(w->text), 6 RG 6 GB 6);

	b = &b0;
	buf_subRect(&w->b, b, w->b.nx - WMTOP, 0, w->b.nx, WMTOP);
	buf_fillRect(b, 3, 3, WMTOP - 3, WMTOP - 3, 6 RG 6 GB 6);
	buf_fillRect(b, 3, WMTOP - 5, WMTOP - 3, WMTOP - 3, 4 RG 4 GB 4);
	buf_fillRect(b, WMTOP - 5, WMTOP - 3, WMTOP - 3, 3, 4 RG 4 GB 4);
	buf_fillRect(b, 3, 3, WMTOP - 3, 4, 7 RG 7 GB 7);
	buf_fillRect(b, 3, WMTOP - 3, 4, 3, 7 RG 7 GB 7);
	buf_fillRect(b, 3, WMTOP - 4, WMTOP - 3, WMTOP - 3, 0 RG 0 GB 0);
	buf_fillRect(b, WMTOP - 4, WMTOP - 3, WMTOP - 3, 3, 0 RG 0 GB 0);

	for (i = 6; i < WMTOP - 6; i++)
		buf_setPixel(b, i, WMTOP - 1 - i, 2 RG 2 GB 2);
	for (i = 6; i < WMTOP - 6; i++)
		buf_setPixel(b, i, i - 1, 0 RG 0 GB 0);
	for (i = 5; i < WMTOP - 7; i++)
		buf_setPixel(b, i, i + 1, 0 RG 0 GB 0);
	for (i = 5; i < WMTOP - 7; i++)
		buf_setPixel(b, i, WMTOP - 3 - i, 0 RG 0 GB 0);
	for (i = 5; i < WMTOP - 6; i++)
		buf_setPixel(b, i, i, 0 RG 0 GB 0);
	for (i = 5; i < WMTOP - 6; i++)
		buf_setPixel(b, i, WMTOP - 2 - i, 0 RG 0 GB 0);
	buf_setPixel(b, WMTOP - 7, WMTOP - 7, 2 RG 2 GB 2);
}

static void SetWindowText(struct Window *w, const char *text)
{
	if (text) {
		if (w->text)
			free(w->text);
		w->text = strndup(text, 32);
	}

	if ((w->flags & 0xff) == WT_CAPTION) {
		draw_caption(w);

	} else if ((w->flags & 0xff) == WT_BUTTON) {
		if (w->isdown)
			draw_button_down(w);
		else
			draw_button_up(w);

	} else if ((w->flags & 0xff) == WT_LABEL) {
		draw_label(w);
	}
}

static struct Window *CreateWindow(struct Window *parent,
		int x0, int y0, int nx, int ny, int zindex, int flags)
{
	struct Window *w, **pp;
	w = CreateHandle(HT_Window, sizeof(struct Window));

	if ((flags & 0xff) == WT_CAPTION) {
		nx += 2;
		ny += WMTOP;
	}

	w->parent = parent;
	if (parent) {
		pp = &parent->children;
		while (*pp) {
			if ((*pp)->zindex >= zindex)
				break;
			pp = &(*pp)->next;
		}
		if ((w->next = (*pp)))
			w->next->pprev = &w->next;
		w->pprev = pp;
		*pp = w;
	}
	w->x0 = x0;
	w->y0 = y0;
	w->flags = flags;
	w->zindex = zindex;
	buf_create(&w->b, nx, ny);

	w->cx0 = w->cy0 = 0;
	w->cx1 = w->b.nx;
	w->cy1 = w->b.ny;

	if ((w->flags & 0xff) == WT_CAPTION) {
		w->cx0 = 1;
		w->cy0 = WMTOP - 1;
		w->cx1 = w->b.nx - 1;
		w->cy1 = w->b.ny - 1;
	}
	SetWindowText(w, NULL);
	return w;
}

static struct Window *JustUpdateWindow(struct Window *w)
{
	struct buf pcb;
	struct Window *parent;
	for (; (parent = w->parent); w = parent) {
		buf_subRect(&parent->b, &pcb,
				parent->cx0, parent->cy0,
				parent->cx1, parent->cy1);
		do {
			buf_blitSub(&pcb, &w->b, w->x0, w->y0);
		} while ((w = w->next));
	}
	return w;
}

static void UpdateWindow(struct Window *w)
{
	w = JustUpdateWindow(w);
	if (w == g_desktop)
		UpdateScreen();
}

#if 0
static int HasWindowIntersection(struct Window *u, struct Window *v)
{
}
#endif

static void EraseWindow(struct Window *w)
{
	struct buf pcb;
	struct Window *u, *parent = w->parent;
	buf_subRect(&parent->b, &pcb,
			parent->cx0, parent->cy0,
			parent->cx1, parent->cy1);
	if (parent->dcb) {
		buf_blitSubSub(&pcb, parent->dcb,
				w->x0, w->y0, w->x0, w->y0,
				w->x0 + w->b.nx, w->y0 + w->b.ny);
	} else {
		buf_fillRect(&pcb, w->x0, w->y0,
				w->x0 + w->b.nx, w->y0 + w->b.ny, 0);
	}
	for (u = parent->children; u && u != w; u = u->next) {
		// if (HasWindowIntersection(u, v))
		buf_blitSub(&parent->b, &u->b, u->x0, u->y0);
	}
}

static void RefreshWindow(struct Window *w, int deep)
{
	struct Window *u;
	struct buf pcb;
	SetWindowText(w, NULL);
	buf_subRect(&w->b, &pcb, w->cx0, w->cy0, w->cx1, w->cy1);
	for (u = w->children; u; u = u->next) {
		if (deep) RefreshWindow(u, deep);
		buf_blitSub(&pcb, &u->b, u->x0, u->y0);
	}
}

static void DestroyWindow(struct Window *w)
{
	EraseWindow(w);
	if (w->text) free(w->text);
	buf_destroy(&w->b);
	if (w->next)
		w->next->pprev = w->pprev;
	*w->pprev = w->next;
	DestroyHandle(w);
}

static void SendMessage(struct Window *w, struct Message *msg)
{
	if (!w->listener)
		return;
	msg->hwnd = w->h.hint;
	ListenerCallback(w->listener, msg);
}

static void on_window_lbutton(struct Window *w, int isdown, int x, int y)
{
	if (!(w->flags & WF_CLICK))
		return;
	struct Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = isdown ? WM_MOUSE_LDOWN : WM_MOUSE_LUP;
	msg.pos.x = x;
	msg.pos.y = y;
	w->isdown = isdown;
	RefreshWindow(w, 0);
	UpdateWindow(w);
	SendMessage(w, &msg);
}

static void SetWindowPos(struct Window *w, int x0, int y0)
{
	EraseWindow(w);
	w->x0 = x0;
	w->y0 = y0;
	UpdateWindow(w);
}

static int IsPointInWindowRect(struct Window *w, int x, int y)
{
	return x >= w->x0 && y >= w->y0 &&
		x < w->x0 + w->b.nx && y < w->y0 + w->b.ny;
}

static struct Window *FindWindowUnder(struct Window *w,
		int *px, int *py, struct Window **mov)
{
	int x = *px, y = *py;
	x -= w->cx0; y -= w->cy0;
	struct Window *u, *r = NULL;
	for (u = w->children; u; u = u->next) {
		if (u->flags & WF_NOSEL)
			continue;
		if (!IsPointInWindowRect(u, x, y))
			continue;
		r = u;
	}
	if (r) {
		x -= r->x0; y -= r->y0;
		if (r->flags & WF_MOVE)
			*mov = r;
		r = FindWindowUnder(r, &x, &y, mov);
	}
	if (!r && !(w->flags & WF_NOSEL))
		r = w;
	if (r) {
		*px = x;
		*py = y;
	}
	return r;
}

static void do_XCreateWindow(int *hwnd, int hparent,
		int x0, int y0, int nx, int ny, int flags)
{
	struct Window *w, *parent = g_desktop;
	if (hparent) {
		parent = GetHandle(hparent, HT_Window);
		if (!parent) return;
	}
	w = CreateWindow(parent, x0, y0,
			nx, ny, 1, flags);
	*hwnd = w->h.hint;
}

static void do_XSetWindowPos(int hwnd, int x0, int y0)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	SetWindowPos(w, x0, y0);
}

static void do_XSetWindowText(int hwnd, const char *text)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	SetWindowText(w, text);
}

static void do_XDestroyWindow(int hint)
{
	struct Window *w = GetHandle(hint, HT_Window);
	if (!w) return;
	DestroyWindow(w);
}

static void do_XUpdateWindow(int hwnd)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	UpdateWindow(w);
}

static void do_XRefreshWindow(int hwnd, int deep)
{
	struct Window *w = g_desktop;
	if (hwnd) {
		w = GetHandle(hwnd, HT_Window);
		if (!w) return;
	}
	RefreshWindow(w, deep);
}

static void do_XListenerBind(int hlst, int hwnd, int deep)
{
	struct Listener *l = GetHandle(hlst, HT_Listener);
	if (!l) return;
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	ListenerBind(l, w, deep);
}

static void desktop_init(void)
{
	g_desktop = CreateWindow(NULL, 0, 0, g_nx, g_ny, 0, WF_NOSEL);
	g_vram = g_desktop->b.rgb;
}

// }}}
// Part of DC: {{{

struct DC {
	struct handle h;
	struct buf b;
	struct Window *window;
	int color, alpha;
};

static struct DC *CreateDC(struct Window *w)
{
	if (w->dcb)
		return NULL;
	struct DC *dc = CreateHandle(HT_DC, sizeof(struct DC));
	//buf_subRect(&w->b, &dc->b, w->cx0, w->cy0, w->cx1, w->cy1);
	dc->window = w;
	buf_create(&dc->b, w->cx1 - w->cx0, w->cy1 - w->cy0);
	w->dcb = &dc->b;
	return dc;
}

static void UpdateDC(struct DC *dc)
{
	struct Window *w = dc->window;
	buf_blitSub(&w->b, &dc->b, w->cx0, w->cy0);
}

static void DestroyDC(struct DC *dc)
{
	buf_destroy(&dc->b);
	dc->window->dcb = NULL;
	DestroyHandle(dc);
}

static void do_XCreateDC(int *hdc, int hwnd)
{
	struct Window *w = GetHandle(hwnd, HT_Window);
	if (!w) return;
	struct DC *dc = CreateDC(w);
	if (!dc) return;
	*hdc = dc->h.hint;
}

static void do_XUpdateDC(int hdc)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	UpdateDC(dc);
}

static void do_XDestroyDC(int hint)
{
	struct DC *dc = GetHandle(hint, HT_DC);
	if (!dc) return;
	DestroyDC(dc);
}

static void do_XSetFillStyle(int hdc, int color)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	dc->color = color;
}

static void do_XFillRect(int hdc, int x0, int y0, int x1, int y1)
{
	struct DC *dc = GetHandle(hdc, HT_DC);
	if (!dc) return;
	buf_fillRect(&dc->b, x0, y0, x1, y1, dc->color);
}

// }}}
// Part of Mouse: {{{

static struct Window *g_mouse;
static int g_mouse_fd;
#define LMB 1
#define MMB 2
#define RMB 4
static int g_mouse_button;
static int g_mx, g_my, g_sel_x, g_sel_y;
static struct Window *g_sel_win, *g_mov_win;

static void on_mouse_lbutton(int isdown)
{
	if (isdown) {
		g_mov_win = NULL;
		g_sel_x = g_mx;
		g_sel_y = g_my;
		g_sel_win = FindWindowUnder(g_desktop,
				&g_sel_x, &g_sel_y, &g_mov_win);
	}
	if (g_sel_win) {
		ssetmask(~0);
		on_window_lbutton(g_sel_win, isdown, g_sel_x, g_sel_y);
		ssetmask(0);
	}
	if (!isdown) {
		g_sel_win = NULL;
		g_mov_win = NULL;
	}
}
 
static void on_mouse_move(int dx, int dy)
{
	static int wx, wy;
	if (g_mov_win) {
		ssetmask(~0);
		SetWindowPos(g_mov_win,
			g_mov_win->x0 + dx,
			g_mov_win->y0 + dy);
		ssetmask(0);
	}
	SetWindowPos(g_mouse, g_mx, g_my);
}

#if 0
void show_mouse_info(void)
{
	printf("mouse %c%c%c %4d %4d\n",
			"L-"[!(g_mouse_button & LMB)],
			"M-"[!(g_mouse_button & MMB)],
			"R-"[!(g_mouse_button & RMB)],
			g_mx, g_my);
}
#endif

static void on_mouse_button_change(int button)
{
	int omb = g_mouse_button;
	g_mouse_button = button & 0x7;
	omb ^= g_mouse_button;
	if (omb & LMB)
		on_mouse_lbutton(g_mouse_button & LMB);
}

static void on_mouse_pos_change(int dx, int dy)
{
	int omx = g_mx, omy = g_my;
	g_mx += dx; g_my += dy;
	buf_pointSanity(&g_desktop->b, &g_mx, &g_my);
	dx = g_mx - omx;
	dy = g_my - omy;
	if (!dx && !dy)
		return;

	on_mouse_move(dx, dy);
}

static void do_mouse(int sig)
{
	char data[3];
	signal(SIGPOLL, do_mouse);
	if (-1 == ionotify(g_mouse_fd, ION_READ))
		perror("cannot ionotify /dev/mouse");
	while (1) {
		data[0] = data[1] = data[2] = 0;
		read(g_mouse_fd, data, 1);
		if (data[0] != 27)
			continue;
		read(g_mouse_fd, data, 3);
		on_mouse_button_change(data[0]);
		if (data[1] || data[2])
			on_mouse_pos_change(data[1], data[2]);
	}
}

static void mouse_init(void)
{
	static char cursor[16][8] = {
		"**......",
		"*O*.....",
		"*O*.....",
		"*OO*....",
		"*OO*....",
		"*OOO*...",
		"*OOO*...",
		"*OOOO*..",
		"*OOOO*..",
		"*OOOOO*.",
		"*OOOOO*.",
		"*OOOOOO*",
		"*OOO****",
		"*O**....",
		"**......",
		"........",
	};

	g_mouse_fd = open("/dev/mouse", O_RDONLY | O_NONBLOCK);
	if (g_mouse_fd == -1) {
		perror("/dev/mouse");
		exit(1);
	}

	ioctl(g_mouse_fd, I_CLBUF);
	signal(SIGPOLL, do_mouse);
	if (-1 == ionotify(g_mouse_fd, ION_READ))
		perror("cannot ionotify /dev/mouse");

	g_mx = g_nx / 2;
	g_my = g_ny / 2;

	g_mouse = CreateWindow(g_desktop, g_mx, g_my,
			8, 16, 256, WF_NOSEL);
	g_mouse->b.col_inv = 0xff;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			RGB c;
			switch (cursor[i][j]) {
			case 'O': c = 6 RG 6 GB 6; break;
			case '*': c = 0 RG 0 GB 0; break;
			default: c = g_mouse->b.col_inv; break;
			};
			buf_setPixel(&g_mouse->b, j, i, c);
		}
	}
	UpdateWindow(g_mouse);
}

// }}}
// Part of IPC: {{{

static int g_msq, g_msq_r;

static void ipc_init(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1) {
		perror("/dev/fb0");
		exit(1);
	}
	key_t key_r = ftok("/dev/fb0", 6666);
	key_t key_m = ftok("/dev/fb0", 8888);

	g_msq = msgget(key, IPC_CREAT | IPC_EXCL);
	if (g_msq == -1) {
		perror("cannot create msqueue for X server");
		exit(1);
	}
	g_msq_r = msgget(key_r, IPC_CREAT | IPC_EXCL);
	if (g_msq_r == -1) {
		perror("cannot create replying msqueue for X server");
		exit(1);
	}
	g_msq_m = msgget(key_m, IPC_CREAT | IPC_EXCL);
	if (g_msq_m == -1) {
		perror("cannot create transfering msqueue for X server");
		exit(1);
	}
}

// }}}

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

	ipc_init();
	screen_init();
	desktop_init();
	mouse_init();

	while (1) {
#include <idl/rax.svr.c>
	}
	return 0;
}
#endif

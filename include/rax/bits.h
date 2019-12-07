#ifndef _RAX_BITS_H
#define _RAX_BITS_H 1

#define WT_CAPTION	1
#define WT_BUTTON	2
#define WT_LABEL	3
#define WF_MOVE		0x100
#define WF_NOSEL	0x200
#define WF_CLICK	0x400

#define WM_MOUSE_LUP	1
#define WM_MOUSE_LDOWN	2
#define WM_CLICK	WM_MOUSE_LUP

struct Message {
	long hlst;
	int hwnd, type;
	union {
		int param[3];
		struct {
			int x, y;
		} pos;
	};
};

#endif

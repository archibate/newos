	struct msg {
		long cmd;
		union msg_un {
			struct a1 {
				int hwnd;
			} a1;
			struct a2 {
				int hdc;
			} a2;
			struct a3 {
				int hdc;
			} a3;
			struct a4 {
				int hdc;
				int bgcolor;
				int fgcolor;
			} a4;
			struct a5 {
				int hdc;
				int x0;
				int y0;
				int x1;
				int y1;
			} a5;
			struct a6 {
				int hdc;
				int x0;
				int y0;
				char text[33];
				int count;
			} a6;
			struct a7 {
				int hdc;
				int x0;
				int y0;
			} a7;
			struct a8 {
				int hparent;
				int x0;
				int y0;
				int nx;
				int ny;
				int flags;
			} a8;
			struct a9 {
				int hwnd;
			} a9;
			struct a10 {
				int hwnd;
			} a10;
			struct a11 {
				int hwnd;
				int deep;
			} a11;
			struct a12 {
				int hwnd;
				char text[33];
			} a12;
			struct a13 {
				int hwnd;
				int x0;
				int y0;
			} a13;
			struct a14 {
			} a14;
			struct a15 {
				int hlst;
			} a15;
			struct a16 {
				int hlst;
				int hwnd;
				int deep;
			} a16;
		} u;	
	} m;
	memset(&m, 0, sizeof(m));
	while (-1 == msgrcv(g_msq, &m, sizeof(m) - sizeof(m.cmd), 0, MSG_NOERROR));
	switch (m.cmd & 0xffff) {
	case 1:
		;struct r1 {
			long seq;
			int hdc;
		} r1;
		do_XCreateDC(&r1.hdc, m.u.a1.hwnd);
		msgsnd(g_msq_r, &r1, sizeof(r1) - sizeof(r1.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 2:
		do_XUpdateDC(m.u.a2.hdc);
		break;
	case 3:
		do_XDestroyDC(m.u.a3.hdc);
		break;
	case 4:
		do_XSetFillStyle(m.u.a4.hdc, m.u.a4.bgcolor, m.u.a4.fgcolor);
		break;
	case 5:
		do_XFillRect(m.u.a5.hdc, m.u.a5.x0, m.u.a5.y0, m.u.a5.x1, m.u.a5.y1);
		break;
	case 6:
		do_XTextOut(m.u.a6.hdc, m.u.a6.x0, m.u.a6.y0, m.u.a6.text, m.u.a6.count);
		break;
	case 7:
		do_XSetPixel(m.u.a7.hdc, m.u.a7.x0, m.u.a7.y0);
		break;
	case 8:
		;struct r8 {
			long seq;
			int hwnd;
		} r8;
		do_XCreateWindow(&r8.hwnd, m.u.a8.hparent, m.u.a8.x0, m.u.a8.y0, m.u.a8.nx, m.u.a8.ny, m.u.a8.flags);
		msgsnd(g_msq_r, &r8, sizeof(r8) - sizeof(r8.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 9:
		do_XUpdateWindow(m.u.a9.hwnd);
		break;
	case 10:
		do_XDestroyWindow(m.u.a10.hwnd);
		break;
	case 11:
		do_XRefreshWindow(m.u.a11.hwnd, m.u.a11.deep);
		break;
	case 12:
		do_XSetWindowText(m.u.a12.hwnd, m.u.a12.text);
		break;
	case 13:
		do_XSetWindowPos(m.u.a13.hwnd, m.u.a13.x0, m.u.a13.y0);
		break;
	case 14:
		;struct r14 {
			long seq;
			int hlst;
		} r14;
		do_XCreateListener(&r14.hlst);
		msgsnd(g_msq_r, &r14, sizeof(r14) - sizeof(r14.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 15:
		do_XDestroyListener(m.u.a15.hlst);
		break;
	case 16:
		do_XListenerBind(m.u.a16.hlst, m.u.a16.hwnd, m.u.a16.deep);
		break;
	}

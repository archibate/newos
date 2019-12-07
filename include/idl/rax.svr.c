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
				int color;
			} a4;
			struct a5 {
				int hdc;
				int x0;
				int y0;
				int x1;
				int y1;
			} a5;
			struct a6 {
				int hparent;
				int x0;
				int y0;
				int nx;
				int ny;
				int flags;
			} a6;
			struct a7 {
				int hwnd;
			} a7;
			struct a8 {
				int hwnd;
			} a8;
			struct a9 {
				int hwnd;
				int deep;
			} a9;
			struct a10 {
				int hwnd;
				char text[33];
			} a10;
			struct a11 {
				int hwnd;
				int x0;
				int y0;
			} a11;
			struct a12 {
			} a12;
			struct a13 {
				int hlst;
			} a13;
			struct a14 {
				int hlst;
				int hwnd;
				int deep;
			} a14;
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
		do_XSetFillStyle(m.u.a4.hdc, m.u.a4.color);
		break;
	case 5:
		do_XFillRect(m.u.a5.hdc, m.u.a5.x0, m.u.a5.y0, m.u.a5.x1, m.u.a5.y1);
		break;
	case 6:
		;struct r6 {
			long seq;
			int hwnd;
		} r6;
		do_XCreateWindow(&r6.hwnd, m.u.a6.hparent, m.u.a6.x0, m.u.a6.y0, m.u.a6.nx, m.u.a6.ny, m.u.a6.flags);
		msgsnd(g_msq_r, &r6, sizeof(r6) - sizeof(r6.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 7:
		do_XUpdateWindow(m.u.a7.hwnd);
		break;
	case 8:
		do_XDestroyWindow(m.u.a8.hwnd);
		break;
	case 9:
		do_XRefreshWindow(m.u.a9.hwnd, m.u.a9.deep);
		break;
	case 10:
		do_XSetWindowText(m.u.a10.hwnd, m.u.a10.text);
		break;
	case 11:
		do_XSetWindowPos(m.u.a11.hwnd, m.u.a11.x0, m.u.a11.y0);
		break;
	case 12:
		;struct r12 {
			long seq;
			int hlst;
		} r12;
		do_XCreateListener(&r12.hlst);
		msgsnd(g_msq_r, &r12, sizeof(r12) - sizeof(r12.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 13:
		do_XDestroyListener(m.u.a13.hlst);
		break;
	case 14:
		do_XListenerBind(m.u.a14.hlst, m.u.a14.hwnd, m.u.a14.deep);
		break;
	}

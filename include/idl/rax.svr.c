	struct msg {
		long cmd;
		union msg_un {
			struct a1 {
				int hwnd;
			} a1;
			struct a2 {
				int hint;
			} a2;
			struct a3 {
				int hdc;
				int color;
			} a3;
			struct a4 {
				int hdc;
				int x0;
				int y0;
				int x1;
				int y1;
			} a4;
			struct a5 {
				int hparent;
				int x0;
				int y0;
				int nx;
				int ny;
				int flags;
			} a5;
			struct a6 {
				int hwnd;
			} a6;
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
		do_XGetDC(&r1.hdc, m.u.a1.hwnd);
		msgsnd(g_msq_r, &r1, sizeof(r1) - sizeof(r1.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 2:
		do_XDestroyDC(m.u.a2.hint);
		break;
	case 3:
		do_XSetFillStyle(m.u.a3.hdc, m.u.a3.color);
		break;
	case 4:
		do_XFillRect(m.u.a4.hdc, m.u.a4.x0, m.u.a4.y0, m.u.a4.x1, m.u.a4.y1);
		break;
	case 5:
		;struct r5 {
			long seq;
			int hwnd;
		} r5;
		do_XCreateWindow(&r5.hwnd, m.u.a5.hparent, m.u.a5.x0, m.u.a5.y0, m.u.a5.nx, m.u.a5.ny, m.u.a5.flags);
		msgsnd(g_msq_r, &r5, sizeof(r5) - sizeof(r5.seq), IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);
		break;
	case 6:
		do_XUpdateWindow(m.u.a6.hwnd);
		break;
	}

#ifdef _VIDEO
#include "busybox.h"
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/notify.h>
#include <rax/rax.h>

static int ptmx, hdc, hwnd, over;

#define CON_COLS 80
#define CON_ROWS 25
#define CON_SIZE (CON_COLS * CON_ROWS)
const int xWin = CON_COLS * 8, yWin = CON_ROWS * 16;

static int con_pos, hide_cur;
static char con_buf[CON_SIZE];

static void con_update(void)
{
	int i, j;
	XSetFillStyle(hdc, 0, 0xc0c0c0);
	XFillRect(hdc, 0, 0, xWin, yWin);
	for (i = 0; i < CON_ROWS; i++) {
		XTextOut(hdc, 0, i * 16, con_buf + i * CON_COLS, CON_COLS);
	}
	int cx = (con_pos % CON_COLS) * 8, cy = (con_pos / CON_COLS) * 16;
	if (!hide_cur) XSetFillStyle(hdc, 0xc0c0c0, 0);
	XFillRect(hdc, cx, cy, cx + 8, cy + 16);
	XTextOut(hdc, 0, cy * 16, con_buf + con_pos, 1);
	XUpdateDC(hdc);
	XUpdateWindow(hwnd);
}

static void on_alarm(int sig)
{
	signal(SIGALRM, on_alarm);
	alarm(1);

	hide_cur = !hide_cur;
	con_update();
}

static void con_putc(int c)
{
	static int state, num, last_num;
	int count, start;
	c &= 0xff;

	hide_cur = 0;

	if (!state)
		num = last_num = 0;

	if (state == 2) {
		switch (c) {
		case '?':
		case '[':
			state = 3;
			return;
		case 'A':
			con_pos -= CON_COLS;
			break;
		case 'B':
			con_pos += CON_COLS;
			break;
		case 'C':
			con_pos--;
			break;
		case 'D':
			con_pos++;
			break;
		case 'K':
			start = con_pos;
			count = 0;
			switch (num) {
			case 0:
				count = CON_COLS - con_pos % CON_COLS;
				break;
			case 1:
				start -= con_pos % CON_COLS;
				count = (con_pos + CON_COLS - 1) % CON_COLS + 1;
				break;
			case 2:
			case 3:
				start -= con_pos % CON_COLS;
				count = CON_COLS;
				break;
			}
fill:
			while (count--) {
				con_buf[start++] = ' ';
			}
			break;
		case 'J':
			start = 0;
			count = 0;
			switch (num) {
			case 0:
				start = con_pos;
				count = CON_SIZE - con_pos;
				break;
			case 1:
				count = con_pos;
				break;
			case 2:
			case 3:
				count = CON_SIZE;
				break;
			}
			goto fill;
		case 'm':
			/* Color not supported for xterm now */
			break;
		case 'H':
			if (!last_num) {
				last_num = num;
				num = 1;
			}
			if (!last_num) last_num = 1;
			con_pos = (last_num - 1) * CON_COLS + (num - 1);
			break;
		case 'h':
		case 'l':
			/* Hide / Show not supported for now */
			break;
		case ';':
			last_num = num;
			num = 0;
			return;
		default:
			if (isdigit(c)) {
				num = num * 10 + (c - '0');
				return;
			}
		};

	} else if (state == 1) {
		switch (c) {
		case '[':
			state = 2;
			return;
		};

	} else {
		switch (c) {
		case '\b':
			if (con_pos > 0)
				con_buf[--con_pos] = ' ';
			break;
		case '\n':
			con_pos += CON_COLS;
			__attribute__((fallthrough));
		case '\r':
			con_pos -= con_pos % CON_COLS;
			break;
		case '\t':
			for (int end = con_pos + (con_pos + 1) % 8;
					con_pos < end; con_pos++)
				con_buf[con_pos++] = ' ';
			break;
		case 27:
			state = 1;
			return;
		default:
			con_buf[con_pos++] = c;
		}
	}

	state = 0;

	while (con_pos < 0)
		con_pos += CON_COLS;
	if (con_pos >= CON_SIZE) {
		memmove(con_buf, con_buf + CON_COLS, CON_SIZE - CON_COLS);
		for (int i = CON_SIZE - CON_COLS; i < CON_SIZE; i++)
			con_buf[i] = ' ';
		con_pos -= CON_COLS;
	}
}

static void con_write(const char *buf, size_t size)
{
	while (size--)
		con_putc(*buf++);
}

static void on_child(int sig)
{
	int stat = 0;
	wait(&stat);
	printf("xterm: child exited %#x\n", stat);
	over = 1;
}

static void open_child_shell(char *const *argv)
{
	static char *defl_argv[] = {"sh", NULL};
	pid_t pid;
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		// todo: ptsname
		int fd = open("/dev/pts/0", O_RDWR | O_CLOEXEC);
		if (fd == -1) {
			perror("/dev/pts/0");
			exit(1);
		}
		dup2(fd, 0);
		dup2(fd, 1);
		fcntl(0, F_SETFL, O_RDONLY | O_CLOEXEC);
		fcntl(1, F_SETFL, O_WRONLY | O_CLOEXEC);
		dup2(1, 2);
		if (!argv[0])
			argv = defl_argv;
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(1);
	}
	printf("xterm: child started %d\n", pid);
}

static void do_message(struct Message *msg)
{
	char data[1];
	if (msg->hwnd != hwnd)
		return;
	//printf("do_message beg\n");
	ssetmask(-1);
	switch (msg->type) {
	case WM_KEYDOWN:
		data[0] = msg->key;
		//printf("in [%c]\n", data[0]);
		if (write(ptmx, data, 1) != 1)
			perror("cannot write /dev/ptmx");
		break;
	/*case WM_DESTROY:
		kill(0, SIGKILL);
		break;*/
	}
	ssetmask(0);
	//printf("do_message end\n");
}

static void do_output(int sig)
{
	//printf("do_output beg\n");
	ssetmask(-1);
	signal(SIGPOLL, do_output);
	ionotify(ptmx, ION_READ, 1);
	char buf[233];
	ssize_t ret = read(ptmx, buf, sizeof(buf) - 1);
	if (ret <= 0) {
		perror("cannot read /dev/ptmx");
		return;
	}
	buf[ret] = 0;
	//printf("out [%.*s]\n", (int)ret, buf);
	con_write(buf, ret);
	con_update();
	ssetmask(0);
	//printf("do_output end\n");
}

static void init_terminal(void)
{
	XFillRect(hdc, 0, 0, xWin, yWin);
	XUpdateDC(hdc);
}

int main(int argc, char **argv)
{
	int hmon, ret;
	struct Message msg;

	while (XClientInit() == -1) {
		perror("cannot connect X server");
	}

	ssetmask(-1);
	XCreateWindow(&hwnd, 0, 30, 30, xWin, yWin, WT_CAPTION | WF_MOVE | WF_KEYDOWN);
	XSetWindowText(hwnd, "Terminal");
	XCreateDC(&hdc, hwnd);
	init_terminal();
	XUpdateWindow(hwnd);

	XCreateListener(&hmon);
	XListenerBind(hmon, hwnd, 1);

	signal(SIGCHLD, on_child);
	open_child_shell(argv + 1);
	ptmx = open("/dev/ptmx", O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (-1 == ptmx) {
		perror("/dev/ptmx");
		return 1;
	}
	signal(SIGPOLL, do_output);
	if (-1 == ionotify(ptmx, ION_READ, 1)) {
		perror("cannot ionotify /dev/ptmx");
		return 1;
	}
	signal(SIGALRM, on_alarm);
	alarm(1);

	ssetmask(0);

	while (!over) {
		memset(&msg, 0, sizeof(msg));
		if (-1 == XListen(hmon, &msg)) {
			perror("XListen");
			continue;
		}
		do_message(&msg);
	}
	XDestroyDC(hdc);
	XDestroyWindow(hwnd);
	XDestroyListener(hmon);

	return 0;
}
#endif

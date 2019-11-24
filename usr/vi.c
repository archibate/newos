#if 0 // sh-stub {{{
true /*
chmod +x $0
if [ ! -z $GDB ]; then C+=$C"-gstabs+ -ggdb -D_DEBUG"; fi
gcc $C -Werror -D_ARGV0=\"$0\" $0 -o /tmp/$$ && $GDB /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>

#ifdef _DEBUG
//#define _LOG
#endif

#ifdef _LOG
FILE *stdlog;
#define dprintf(...) fprintf(stdlog, __VA_ARGS__)
#else
#define dprintf(...)
#endif

#define NX 80
#define NY 24

#define MAX_NX (2*NX)
#define MAX_NY (2*NY)

#define TABS 8

int getkb(void)
{
	int c = getchar();
	return c;
}

int editing, cmd_mode, cx, cy;
char *text, *textend, *top, *dot, *end;

// ANSI control sequences {{{ 

void gotoy0(int y)
{
	if (y < 0) y = 0;
	if (y >= NY) y = NY - 1;
	printf("\033[%dH", y + 1);
}

void gotoyx(int y, int x)
{
	if (y < 0) y = 0;
	if (y >= NY) y = NY - 1;
	if (x < 0) x = 0;
	if (x >= NX) x = NX - 1;
	printf("\033[%d;%dH", y + 1, x + 1);
}

void ceol(void)
{
	printf("\033[0K");
}

void ceos(void)
{
	printf("\033[0J");
}

void emcolor(void)
{
	printf("\033[1m");
}

void fgcolor(int c)
{
	printf("\033[3%dm", c & 7);
}

void bgcolor(int c)
{
	printf("\033[4%dm", c & 7);
}

void standup(void)
{
	printf("\033[7m");
}

void sitdown(void)
{
	printf("\033[0m");
}

void cmup(void)
{
	printf("\033[A");
}

void cmdown(void)
{
	printf("\033[B");
}

void cmbol(void)
{
	putchar('\r');
}

void cmdown0(void)
{
	putchar('\n');
}

void beep(void)
{
	putchar('\a');
}

// }}}
// switch to rawmode {{{

struct termios tc_orig, tc_vi;

void cookmode(void)
{
	sitdown();
	gotoy0(NY);
	putchar('\n');
	tcsetattr(0, TCSANOW, &tc_orig);
}

void rawmode(void)
{
	if (tcgetattr(0, &tc_orig) == -1) {
		fprintf(stderr, "vi: input not from terminal, exiting\n");
		exit(1);
	}
	memcpy(&tc_vi, &tc_orig, sizeof(struct termios));
	tc_vi.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &tc_vi);
	atexit(cookmode);
	signal(SIGINT, exit);
}

// }}}
// locating operations {{{

char *begin_line(char *p)
{
	while (p > text && p[-1] != '\n')
		p--;
	return p;
}

char *end_line(char *p)
{
	while (p < end - 1 && *p != '\n')
		p++;
	return p;
}

char *dollar_line(char *p)
{
	while (*p != '\n')
		p++;
	if (*p == '\n' && p > text && p[-1] != '\n')
		p--;
	return p;
}

char *prev_line(char *p)
{
	p = begin_line(p);
	if (p > text && p[-1] == '\n')
		p--;
	p = begin_line(p);
	return p;
}

char *next_line(char *p)
{
	char *q = end_line(p);
	if (q < end - 1)
		p = q + 1;
	return p;
}

char *end_screen(void)
{
	int i;
	char *p = top;
	for (i = 0; i < NY - 1; i++)
		p = next_line(p);
	p = end_line(p);
	return p;
}

char *skip_spaces(char *p)
{
	while (isblank(*p))
		*p++;
	return p;
}

// aaa  bbbb cccc--cc__--cc  ddd

char *next_bigword(char *p)
{
	while (!isspace(*p))
		p++;
	while (isspace(*p))
		p++;
	return p;
}

char *prev_bigword(char *p)
{
	while (isspace(*p))
		p--;
	while (!isspace(*p))
		p--;
	return p;
}
// }}}
// screen display {{{

char status_bar[100];

void error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	strcpy(status_bar, "\033[7m");
	vsprintf(status_bar + strlen(status_bar), fmt, ap);
	strcat(status_bar, "\033[0m");
	va_end(ap);
}

void format_line(const char *s, int line)
{
	int x, c;
	for (x = 0; x < MAX_NX; x++) {
		c = ' ';
		if (line > 0 && x == 0)
			c = '~';
		if (s < end)
			c = *s++;
		if (c == '\n')
			break;
		if (!isprint(c)) {
			if (c == '\t') {
				c = ' ';
				for (; x % TABS != TABS - 1; x++)
					putchar(c);
			} else {
				x++;
				putchar('^');
				c |= '@';
				c &= 0x7f;
			}
		}
		putchar(c);
		if (s >= end)
			break;
	}
}

void sync_cursor(void)
{
	int cnt, y, x = 0;
	char *tp, *tmp, *es = end_screen();
	//dprintf("dot=%.4s\n", dot);
	if (dot < top) {
		y = 0;
		while (dot < top)
			top = prev_line(top);

	} else if (dot > es) {
		while (dot > es) {
			top = next_line(top);
			es = next_line(es);
			es = end_line(es);
		}
		y = NY - 1;

	} else {
		tp = top;
		for (y = 0; y < NY; y++) {
			tmp = end_line(tp);
			if (tmp >= end - 1 || tmp >= dot)
				break;
			tp = tmp + 1;
		}

		do {
			if (*tp == '\n' || !*tp)
				break;
			if (*tp == '\t')
				x += TABS - 1 - x % TABS;
			else if (!isprint(*tp))
				x++;
		} while (tp++ < dot && ++x);
	}
	if (x < 0) x = 0;
	if (x >= NX) x = NX - 1;
	if (y < 0) y = 0;
	if (y >= NY) y = NY - 1;
	cx = x;
	cy = y;
}

void redraw(void)
{
	int y;
	char *tp;
	sync_cursor();
	tp = top;
	gotoy0(0);
	ceos();
	for (y = 0; y < NY; y++) {
		format_line(tp, y);
		while (tp < end && *tp++ != '\n');
		cmdown0();
	}
	printf("%s", status_bar);
	gotoyx(cy, cx);
}

// }}}
// text operations {{{

void make_hole(char *p, int size)
{
	memmove(p + size, p, end - p);
	memset(p, ' ', size);
	end += size;
}

char *remove_hole(char *p, char *q)
{
	int hole, cnt;
	char *dst, *src;
	if (p <= q) {
		src = q + 1;
		dst = p;
	} else {
		src = p + 1;
		dst = q;
	}
	hole = q - p + 1;
	cnt = end - src;
	if (src < text || src > end)
		return dst;
	if (dst < text || dst >= end)
		return dst;
	memmove(dst, src, cnt);
	end -= hole;
	if (dst >= end)
		dst = end - 1;
	if (end <= text)
		dst = end = text;
	return dst;
}

char *just_insert_char(char *p, int c)
{
	make_hole(p, 1);
	*p++ = c;
	return p;
}

char *insert_char(char *p, int c)
{
	if (c == 27) {
		cmd_mode = 0;
		*status_bar = 0;
		if (p > text && p[-1] != '\n')
			p--;
	} else if (c == 22) { // ^V
		just_insert_char(p, '^');
		redraw();
		c = getkb();
		*p++ = c;
	} else if (c == tc_vi.c_cc[VERASE]) {
		if (p > text && p[-1] != '\n') {
			p--;
			p = remove_hole(p, p);
		}
	} else {
		if (c == '\r')
			c = '\n';
		p = just_insert_char(p, c);
	}
	return p;
}

// }}}
// command impelementation {{{

void dot_left(unsigned n)
{
	while (n-- && dot > text && dot[-1] != '\n')
		dot--;
}

void dot_right(unsigned n)
{
	while (n-- && dot < end - 1 && dot[1] != '\n')
		dot++;
}

void dot_dollar(unsigned n)
{
	while (n-- > 1)
		dot = next_line(dot);
	dot = dollar_line(dot);
}

void dot_begin_skipws(unsigned n)
{
	dot = begin_line(dot);
	dot = skip_spaces(dot);
}

void dot_prev_skipws(unsigned n)
{
	while (n--)
		dot = prev_line(dot);
	dot = skip_spaces(dot);
}

void dot_next_skipws(unsigned n)
{
	while (n--)
		dot = next_line(dot);
	dot = skip_spaces(dot);
}

void dot_move_to_col(int col)
{
	int i = 0;
	dot = begin_line(dot);
	do {
		if (*dot == '\n' || !*dot)
			break;
		if (*dot == '\t') {
			i += TABS - 1 - i % TABS;
		} else if (!isprint(*dot)) {
			i++;
		}
	} while (++i <= col && dot++ < end);
}

void dot_down(unsigned n)
{
	while (n--)
		dot = next_line(dot);
	dot_move_to_col(cx);
}

void dot_up(unsigned n)
{
	while (n--)
		dot = prev_line(dot);
	dot_move_to_col(cx);
}

void dot_forward(int c)
{
	char *q = dot + 1;
	while (q < end - 1 && *q != '\n' && *q != c) {
		q++;
	}
	if (*q == c)
		dot = q;
}

void dot_goto_top(unsigned n)
{
	dot = top;
	if (n >= NY)
		n = NY;
	dot_next_skipws(n - 1);
}

void dot_goto_bottom(unsigned n)
{
	dot = end_screen();
	if (n >= NY)
		n = NY;
	dot_prev_skipws(n - 1);
}

void dot_delete(unsigned n)
{
	while (n--)
		remove_hole(dot, dot);
}

void dot_join(unsigned n)
{
	while (n--) {
		dot = end_line(dot);
		if (dot < end - 1) {
			*dot++ = ' ';
			while (isblank(*dot))
				remove_hole(dot, dot);
		}
	}
}

void dot_delchar(unsigned n)
{
	if (*dot == '\n')
		return;
	remove_hole(dot, dot + n - 1);
}

void dot_delte_to_eol(void)
{
	char *q = dollar_line(dot);
	remove_hole(dot, q);
}

void dot_delrange(int c)
{
	/*find_range(&p, &q, c);
	if (c == 27)*/
}

// }}}
// command parser {{{

unsigned cmdcnt;
int forward_char;

void do_cmd(int c)
{
	char *p;
	int cc = cmdcnt ? cmdcnt : 1;
	switch (c) {
	case 'h':
		dot_left(cc);
		break;
	case 'l':
		dot_right(cc);
		break;
	case 'j':
		dot_down(cc);
		break;
	case 'k':
		dot_up(cc);
		break;
	case 'A':
		dot = end_line(dot);
		__attribute__((fallthrough));
	case 'a':
		if (*dot != '\n')
			dot++;
		goto cmd_i;
	case 'I':
		dot_begin_skipws(1);
		__attribute__((fallthrough));
	case 'i':
cmd_i:
		cmd_mode = 1;
		strcpy(status_bar, "-- INSERT --");
		break;
	case 'O':
		p = begin_line(dot);
		if (p[-1] == '\n') {
			dot = prev_line(dot);
	case 'o':
			dot = end_line(dot);
			dot = insert_char(dot, '\n');
		} else {
			dot = begin_line(dot);
			dot = insert_char(dot, '\n');
			dot = prev_line(p);
		}
		goto cmd_i;
	case 'X':
		if (!(dot > text && dot[-1] != '\n'))
			break;
		dot--;
		__attribute__((fallthrough));
	case 'x':
	case 's':
		dot_delchar(cc);
		if (c == 's')
			goto cmd_i;
		break;
	case 'c':
	case 'd':
		dot_delrange(getkb());
		if (c == 'c')
			goto cmd_i;
		break;
	case 'C':
	case 'D':
		dot_delte_to_eol();
		if (c == 'C')
			goto cmd_i;
		break;
	case '+':
		dot_next_skipws(cc);
		break;
	case '-':
		dot_prev_skipws(cc);
		break;
	case '$':
		dot_dollar(cc);
		break;
	case '_':
		dot_begin_skipws(cc);
		break;
	case '^':
		dot_begin_skipws(1);
		break;
	case '|':
		dot_move_to_col(cc - 1);
		break;
	case 'f':
		forward_char = getkb();
		__attribute__((fallthrough));
	case ';':
		if (cmdcnt > 1) {
			cmdcnt--;
			do_cmd(';');
		}
		if (!forward_char)
			break;
		dot_forward(forward_char);
		break;
	case 'H':
		dot_goto_top(cc);
		break;
	case 'L':
		dot_goto_bottom(cc);
		break;
	case 'M':
		dot_goto_top((NY - 1) / 2);
		break;
	case 'J':
		dot_join(cc);
		break;
	case 'Z':
		if (getkb() == 'Z')
			editing = 0;
		break;
	case 27:
		cmdcnt = 0;
		break;
	case '0':
		if (cmdcnt < 1) {
			dot = begin_line(dot);
			break;
		}
		__attribute__((fallthrough));
	default:
		if ('0' <= c && c <= '9')
			cmdcnt = cmdcnt * 10 + (c - '0');
		c = getkb();
		do_cmd(c);
		break;
	}
}

void do_key(int c)
{
	if (cmd_mode == 1) {
		if (isascii(c)) {
			dot = insert_char(dot, c);
		}

	} else if (cmd_mode == 0) {
		cmdcnt = 0;
		do_cmd(c);
	}

	if (end == text) {
		insert_char(text, '\n');
		dot = text;
	}
	if (dot != end) {
		if (dot >= end && end > text)
			dot = end - 1;
		if (dot < text)
			dot = text;
	}
}

// }}}
// body of edit_file {{{

int insert_from_file(const char *path, char *p, int size)
{
	FILE *f;
	int ret;
	if (!path || !strlen(path)) {
		error("No filename given");
		return 1;
	}
	if (size <= 0)
		return 0;

	f = fopen(path, "r+");
	if (!f) {
		f = fopen(path, "r");
		if (!f) {
err:			error("\"%s\" %s", path, strerror(errno));
			return 1;
		}
	}

	make_hole(p, size);
	ret = fread(p, size, 1, f);
	fclose(f);
	if (ret != 1)
		goto err;
	return 0;
}

void new_text(int size)
{
	size *= 2;
	if (size < 2333)
		size = 2333;
	if (text)
		free(text);
	text = calloc(size, 1);
	text += 4;
	textend = text + size - 1;
	textend -= 4;
}

int file_size(const char *path)
{
	struct stat st;
	if (!path || !strlen(path))
		return -1;
	if (stat(path, &st) == -1)
		return -1;
	return st.st_size;
}

void edit_file(const char *path)
{
	int c, ret = 1;
	int size = file_size(path);
	new_text(size * 2);
	top = dot = end = text;

	if (path)
		insert_from_file(path, text, size);
	if (end[-1] != '\n')
		insert_char(text, '\n');

	redraw();

	editing = 1;
	while (editing) {
		c = getkb();
		do_key(c);
		redraw();
	}
}

// }}}

int main(int argc, char **argv)
{
	int i;
	rawmode();
#ifdef _LOG
	stdlog = fopen("/tmp/vi.log", "w");
	if (!stdlog) {
		perror("cannot open log file");
		return 1;
	}
	setbuf(stdlog, NULL);
	fprintf(stdlog, "\033[2J\033[0H\033[3J");
#endif
#ifdef _DEBUG
	edit_file("tools/test.txt");
#else
	if (argc <= 1) {
		edit_file(NULL);
	} else {
		for (i = 1; i < argc; i++)
			edit_file(argv[i]);
	}
#endif
	return 0;
}

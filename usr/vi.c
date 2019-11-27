#if 0 // sh-stub {{{
true /*
chmod +x $0
if [ ! -z $GDB ]; then C+="-gstabs+ -ggdb -D_DEBUG"; fi
gcc $C -Werror -D_ARGV0=\"$0\" $0 -o /tmp/$$ && $GDB /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>

static int isident(int c)
{
	return isalnum(c) || c == '_';
}

static int issymbl(int c)
{
	return !isident(c) && !isspace(c);
}

#ifdef _DEBUG
#define _LOG
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

struct termios tc_orig, tc_vi;
int editing, modified, readonly, newfile, cmd_mode, status_error, cx, cy;
char *text, *undotext, *textend, *top, *dot, *doll, *end;
char status_bar[100];
const char *g_path;

// ANSI control sequences {{{ 

void gotoy0(int y)
{
#if 0
	if (y < 0) y = 0;
	if (y >= NY) y = NY - 1;
#endif
	printf("\033[%dH", y + 1);
}

void gotoyx(int y, int x)
{
#if 0
	if (y < 0) y = 0;
	if (y >= NY) y = NY - 1;
	if (x < 0) x = 0;
	if (x >= NX) x = NX - 1;
#endif
	printf("\033[%d;%dH", y + 1, x + 1);
}

void ceol(void)
{
	printf("\033[K");
}

void ceos(void)
{
	printf("\033[J");
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
// move-in-text operations {{{

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
// }}}
// screen display {{{

#define error(...) do { \
	status_error = 1; \
	sprintf(status_bar, __VA_ARGS__); \
} while (0)

#define info(...) do { \
	status_error = 0; \
	sprintf(status_bar, __VA_ARGS__); \
} while (0)

int getkb(void)
{
	int n, c = getchar();
	return c;
}

char *printable(int c)
{
	static char buf[3];
	if (isprint(c)) {
		buf[0] = c;
		buf[1] = 0;
	} else {
		buf[0] = '^';
		c |= '@';
		c &= 0x7f;
		if (c == 0x7f)
			c = '?';
		buf[1] = c;
		buf[2] = 0;
	}
	return buf;
}

void format_line(const char *s, int line)
{
	int x, c;
	for (x = 0; x < MAX_NX; x++) {
		c = ' ';
		if (line > 0 && line < NY && x == 0)
			c = '~';
		if (s < end)
			c = *s++;
		if (c == "\n"[line == NY])
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
				if (c == 0x7f)
					c = '?';
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
		tp = top;

	} else if (dot > es) {
		while (dot > es) {
			top = next_line(top);
			es = next_line(es);
			es = end_line(es);
		}
		y = NY - 1;
		tp = begin_line(es);

	} else {
		tp = top;
		for (y = 0; y < NY; y++) {
			tmp = end_line(tp);
			if (tmp >= end - 1 || tmp >= dot)
				break;
			tp = tmp + 1;
		}
	}

	do {
		if (*tp == '\n' || !*tp)
			break;
		if (*tp == '\t')
			x += TABS - 1 - x % TABS;
		else if (!isprint(*tp))
			x++;
	} while (tp++ < dot && ++x);

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
	if (status_error)
		standup();
	format_line(status_bar, NY);
	if (status_error)
		sitdown();
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
	char *dst, *src;
	if (p <= q) {
		src = q + 1;
		dst = p;
	} else {
		src = p + 1;
		dst = q;
	}
	int cnt = end - src;
	if (src < text || src > end)
		return dst;
	if (dst < text || dst >= end)
		return dst;
	memmove(dst, src, cnt);
	end -= src - dst;
	if (dst >= end)
		dst = end - 1;
	if (end <= text)
		dst = end = text;
	return dst;
}

char *reserve_hole(char *p, char *q)
{
	char *t;
	if (p > q) {
		t = q;
		q = p;
		p = t;
	}
	doll = q;
	*q = '$';
	return p;
}

char *remove_hole_nx(char *p, char *q, int rsv)
{
	char *t;
	if (p > q) {
		t = q;
		q = p;
		p = t;
	}
	if (*p == '\n')
		return p;
	for (t = p; t < q; t++) {
		if (t[1] == '\n') {
			q = t;
			break;
		}
	}
	if (rsv)
		t = reserve_hole(p, q);
	else
		t = remove_hole(p, q);
	return t;
}

char *just_insert_char(char *p, int c)
{
	if (p > doll)
		make_hole(p, 1);
	*p++ = c;
	return p;
}

char *insert_char(char *p, int c)
{
	if (c == 27) {
		if (p <= doll) {
			remove_hole(p, doll);
			doll = NULL;
		}
		cmd_mode = 0;
		info("");
		if (p > text && p[-1] != '\n')
			p--;
	} else if (c == 22) { // ^V
		just_insert_char(p, '^');
		redraw();
		c = getkb();
		*p++ = c;
	} else if (c == '\b' || c == 0x7f) {
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
// command moving cursor {{{

void dot_left(unsigned n)
{
	while (n-- && dot > text && dot[-1] != '\n')
		dot--;
}

void dot_right(unsigned n)
{
	if (*dot == '\n')
		return;
	while (n-- && dot[1] != '\n')
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
	while (n-- > 1)
		dot = next_line(dot);
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

void dot_forward(int c, int ist, int iscont)
{
	char *q = dot + 1;
	if (iscont && *q != '\n')
		q++;
	while (q < end - 1 && *q != '\n' && *q != c)
		q++;
	if (*q == c) {
		dot = q;
		if (ist)
			dot_left(1);
	}
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
	int i;
	dot = top;
	for (i = 0; i < NY - 1; i++)
		dot = next_line(dot);
	if (n >= NY)
		n = NY;
	dot_prev_skipws(n - 1);
}

void dot_forward_word(unsigned n)
{
	while (n--) {
		if (isident(*dot))
			do
				dot++;
			while (isident(*dot));
		else if (issymbl(*dot))
			do
				dot++;
			while (issymbl(*dot));
		while (*dot != '\n' && isspace(*dot))
			dot++;
		if (dot < end - 1 && *dot == '\n')
			dot++;
	}
}

void dot_back_word(unsigned n)
{
	while (n--) {
		dot--;
		while (dot > text && isspace(*dot) && *dot != '\n')
			dot--;
	 	if (isident(*dot))
			while (dot > text && isident(dot[-1]))
				dot--;
		else if (issymbl(*dot))
			while (dot > text && issymbl(dot[-1]))
				dot--;
	}
}

void dot_end_word(unsigned n)
{
	while (n--) {
		dot++;
		while (dot < end - 1 && isspace(*dot) && *dot != '\n')
			dot++;
		if (dot + 1 < end - 1 && *dot == '\n' && (dot <= text || dot[-1] != '\n'))
			dot++;
	 	if (isident(*dot))
			while (isident(dot[1]))
				dot++;
		else if (issymbl(*dot))
			while (issymbl(dot[1]))
				dot++;
	}
}

void dot_goto_line(unsigned n)
{
	if (!n) {
		dot = begin_line(end - 1);
		return;
	}
	n--;
	for (dot = text; n; dot++)
		if (*dot == '\n')
			n--;
	dot = begin_line(dot);
}

// }}}
// command modifying text {{{

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

void dot_delchar(unsigned n, int rsv)
{
	if (*dot == '\n')
		return;
	remove_hole_nx(dot, dot + n - 1, rsv);
}

void dot_delete_to_eol(int rsv)
{
	char *q = dollar_line(dot);
	remove_hole_nx(dot, q, rsv);
}

int do_cmd(int c);

int find_range(char **start, char **stop, int c, unsigned n)
{
	int i;
	char *old_dot, *p, *q;
	p = q = old_dot = dot;

	if (strchr("eEbBft;%0hl$^", c)) {
		do_cmd(c);
		q = dot;
	} else if (strchr("wW", c)) {
		do_cmd(c);
		if (dot > text)
			dot--;
		if (dot > text && *dot == '\n')
			dot--;
		q = dot;
	} else if (strchr("k-{H", c)) {
		q = end_line(dot);
		do_cmd(c);
		p = dot = begin_line(dot);
	} else if (strchr("j_+}L\n", c)) {
		p = begin_line(dot);
		do_cmd(c);
		q = dot = end_line(dot);
	} else {
		return 0;
	}
	*start = p;
	*stop = q;
	return 1;
}

int dot_delrangec(unsigned n, int c0)
{
	char *p, *q;
	int c = getkb();
	if (c == 27)
		return 0;
	if (c == c0)
		c = '_';
	if (!find_range(&p, &q, c, n)) {
		error("Bad command '%c%s'\n", c0, printable(c));
		return 0;
	}
	if (strchr("wW", c)) {
		if (c0 != 'd') {
			while (isblank(*q)) {
				if (q <= text || q[-1] == '\n')
					break;
				q--;
			}
		}
		dot = remove_hole_nx(p, q, c0 != 'd');
	} else if (strchr("kj-_+%{}HL\n", c)) {
		dot = remove_hole(p, q);
		if (c0 != 'd') {
			dprintf("dot=%d=%s%s\n", dot - text, strdup(printable(dot[-1])), strdup(printable(*dot)), strdup(printable(dot[1])));
			dot = insert_char(dot, '\n');
			dprintf("dot=%d=%s%s%s\n", dot - text, strdup(printable(dot[-1])), strdup(printable(*dot)), strdup(printable(dot[1])));
			if (dot < end - 1)
				dot = prev_line(dot + 1);
			dprintf("dot=%d=%s%s\n", dot - text, strdup(printable(dot[-1])), strdup(printable(*dot)), strdup(printable(dot[1])));
		} else {
			dot_begin_skipws(1);
		}
	} else if (strchr("eEbBft;0hl$^\b\x7f ", c)) {
		dot = remove_hole_nx(p, q, c0 != 'd');
	} else {
		return 0;
	}
	return c0 != 'd';
}

// }}}
// prompt parser {{{

int save_file(const char *path);
void edit_file(const char *path);
int open_file(const char *path);

int matchcmd(char **p, const char *cmd)
{
	char *q = *p;
	while (*q && !isblank(*q) && *q != '!')
		q++;
	if (q - *p > (int)strlen(cmd) ||
		memcmp(*p, cmd, q - *p))
		return 0;
	while (*q && isblank(*q))
		q++;
	*p = *q ? q : NULL;
	return 1;
}

void do_prompt(void)
{
	int c;
	char *op, *p;
	info(":");
	redraw();
	gotoyx(NY, 1);
	while ('\n' != (c = getkb())) {
		if (c == 27) {
			info("");
			redraw();
			return;
		}
		strcat(status_bar, printable(c));
		redraw();
		gotoyx(NY, strlen(status_bar));
	}
	redraw();
	p = strdup(status_bar + 1);
	//op = p;
	if (matchcmd(&p, "open")) {
		open_file(p);
	} else if (matchcmd(&p, "write")) {
		save_file(p);
	} else if (matchcmd(&p, "new")) {
		open_file(NULL);
	} else if (matchcmd(&p, "edit")) {
		edit_file(p);
	} else if (matchcmd(&p, "quit")) {
		editing = 0;
	} else {
		error("Bad command \":%s\"", p);
	}
	//free(op);
}

// }}}
// command parser {{{

unsigned cmdcnt;
int forward_char, forward_is_till;

int do_cmd(int c)
{
	char *p;
	int c1, cc = cmdcnt > 0 ? cmdcnt : 1;
	switch (c) {
	case ':':
		do_prompt();
		c = 27;
		break;
	//case 'Z':
		if (getkb() == 'Z') {
			if (!save_file(NULL))
				editing = 0;
		}
		goto done;
	}
// moving commands {{{
	switch (c) {
	case '\b':
	case 0x7f:
	case 'h':
		dot_left(cc);
		break;
	case ' ':
	case 'l':
		dot_right(cc);
		break;
	case 'j':
		dot_down(cc);
		break;
	case 'k':
		dot_up(cc);
		break;
	case '\n':
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
	case 'w':
		dot_forward_word(cc);
		break;
	case 'b':
		dot_back_word(cc);
		break;
	case 'e':
		dot_end_word(cc);
		break;
	case 't':
	case 'f':
		forward_is_till = c == 't';
		forward_char = getkb();
		__attribute__((fallthrough));
	case ';':
		if (cmdcnt > 1) {
			cmdcnt--;
			do_cmd(';');
		}
		if (!forward_char)
			break;
		dot_forward(forward_char, forward_is_till, c == ';');
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
	case 'g':
		if (getkb() == 'g')
			dot_goto_line(1);
		break;
	case 'G':
		dot_goto_line(cmdcnt);
		break;
	case 'J':
		dot_join(cc);
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
		if (isdigit(c)) {
			cmdcnt = cmdcnt * 10 + (c - '0');
			c = getkb();
			return do_cmd(c);
		} else {
			goto modifying;
		}
		break;
	}
	goto done;
modifying:
// }}}
// modifying commands {{{
	if (readonly) {
		error("\"%s\" File is read only", g_path);
		return 0;
	}
	switch (c) {
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
		info("-- INSERT --");
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
		dot_delchar(cc, c == 's');
		if (c == 's')
			goto cmd_i;
		break;
	case 'v':
		dot_right(1);
		__attribute__((fallthrough));
	case 'c':
	case 'd':
		if (dot_delrangec(cc, c))
			goto cmd_i;
		break;
	case 'S':
		dot = begin_line(dot);
		__attribute__((fallthrough));
	case 'C':
	case 'D':
		dot_delete_to_eol(c == 'C');
		if (c != 'D')
			goto cmd_i;
		break;
	default: bad:
		error("Bad command '%s'", printable(c));
		return 0;
	}
	// }}}
done:
	if (text == end) {
		insert_char(text, '\n');
		dot = text;
	}
	if (cmd_mode == 0 && dot > text && *dot == '\n' && dot[-1] != '\n')
		dot--;
	return 1;
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
	if (dot > end && end > text)
		dot = end - 1;
	if (dot < text)
		dot = text;
}

// }}}
// body of edit_file {{{

void show_file_status(void)
{
	int lines = 0;
	char *p;
	if (!g_path)
		return;
	info("\"%s\"", g_path);
	if (newfile)
		sprintf(status_bar + strlen(status_bar), " [New file]");
	if (readonly)
		sprintf(status_bar + strlen(status_bar), " [Read only]");

	if (!newfile) {
		for (p = text; p < end; p++)
			if (*p == '\n')
				lines++;
		sprintf(status_bar + strlen(status_bar),
				" %d lines, %d characters", lines, end - text);
	}
}

int load_file(const char *path, int size)
{
	FILE *f;
	int ret;
	readonly = newfile = 0;

	if (!path || !strlen(path)) {
		error("No filename given");
		return 1;
	}
	info("\"%s\"", g_path);

	if (size < 0) {
		newfile = 1;
		return 0;
	}

	f = fopen(path, newfile ? "a" : "r");
	if (!f) {
err:		error("\"%s\" %s", path, strerror(errno));
		return 1;
	}

	end = text + size;
	ret = fread(text, size, 1, f);
	fclose(f);
	if (ret != 1)
		goto err;

	return 0;
}

int save_file(const char *path)
{
	FILE *f;
	int ret;
	if (!path) {
		if (readonly) {
			error("\"%s\" File is read only", g_path);
			return 1;
		}
		path = g_path;
	}
	if (!path || !strlen(path)) {
		error("No current filename");
		return 1;
	}
	info("\"%s\"", g_path);

	f = fopen(path, "w");
	if (!f) {
err:		error("\"%s\" Cannot write: %s", path, strerror(errno));
		return 1;
	}
	ret = fwrite(text, end - text, 1, f);
	fclose(f);
	if (ret != 1)
		goto err;

	g_path = path;

	show_file_status();
	redraw();
	return 0;
}

void new_text(int size)
{
	size *= 2;
	if (size < 2333)
		size = 2333;
	if (text)
		free(text - 4);
	if (undotext)
		free(undotext - 4);
	text = calloc(size, 1) + 4;
	undotext = calloc(size, 1) + 4;
	textend = text + size - 5;
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

int open_file(const char *path)
{
	int size = file_size(path);
	new_text(size * 2);
	top = dot = end = text;

	if (path)
		load_file(path, size);
	if (end[-1] != '\n')
		insert_char(text, '\n');

	if (!status_error) {
		g_path = path;
		show_file_status();
	}
	redraw();
	return 0;
}

void edit_file(const char *path)
{
	int c;
	open_file(path);

	editing = 1;
	while (editing) {
		c = getkb();
		do_key(c);
		redraw();
	}

	g_path = NULL;
}

// }}}
// switch to rawmode {{{

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
	fprintf(stdlog, "\033[2J\033[H\033[3J");
#endif
#ifdef _DEBUG
	edit_file("tools/ol.txt");
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

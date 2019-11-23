#if 0 // {{{
true /*
chmod +x $0
gcc -Werror -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#define SCRX 80
#define SCRY 24
#define FRGSIZ 32
#define TABSTOP 8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>

int getkb(void);

struct line
{
	struct line *next, **pprev;
	char *data;
	size_t size;
	size_t bufsiz;
};

struct buf {
	struct line *first;
	struct line *scrn_first;
	struct line *curr;
	size_t cx, cy;
};


struct line *alloc_line(void)
{
	struct line *n;
	n = calloc(sizeof(struct line), 1);
	return n;
}

void free_line(struct line *n)
{
	if (n->data)
		free(n->data);
	free(n);
}

void delete_line(struct line *n)
{
	*n->pprev = n->next;
	free_line(n);
}

struct line *line_inschar(struct line *n, size_t pos, int c)
{
	if (pos > n->size)
		pos = n->size;
	if (n->size + 1 >= n->bufsiz) {
		n->bufsiz += FRGSIZ;
		n->data = realloc(n->data, n->bufsiz);
	}
	memmove(n->data + pos + 1, n->data + pos, n->size - pos);
	n->size++;
	n->data[pos] = c;
	return n;
}

void line_delchar(struct line *n, size_t pos)
{
	if (pos > n->size)
		pos = n->size;
	n->size--;
	memmove(n->data + pos, n->data + pos + 1, n->size - pos);
}

void buf_destroy(struct buf *b)
{
	struct line *n, *next;
	for (n = b->first; n; n = next) {
		next = n->next;
		delete_line(n);
	}
	b->scrn_first = NULL;
	b->first = NULL;
	b->cx = 0;
}

void buf_show(struct buf *b)
{
	int c;
	struct line *n;
	size_t i, j, ax = 1;
	printf("\33[3J\33[H\33[2J");
	for (n = b->scrn_first, j = 0; n && j < SCRY; n = n->next, j++) {
		for (i = 0; i < n->size; i++) {
			int c = n->data[i];
			if (c == '\t') {
				c = TABSTOP + 1 - (i + 1) % TABSTOP;
				if (j == b->cy && i <= b->cx)
					ax += c - 1;
				while (c--)
					putchar(' ');
			} else {
				putchar(c);
			}
		}
		putchar('\n');
	}
	printf("\33[%lu;%luH", b->cy + 1, b->cx + ax);
}

int buf_load(struct buf *b, const char *path)
{
	int c;
	char buf[SCRX + 2];
	struct line *n, **pprev = &b->first;
	FILE *f = fopen(path, "r");
	if (!f) {
		perror(path);
		return -1;
	}
	while (fgets(buf, sizeof(buf), f)) {
		n = alloc_line();
		n->size = strlen(buf);
		if (buf[n->size - 1] == '\n')
			buf[--n->size] = 0;
		n->data = strdup(buf);
		n->bufsiz = n->size + 1;
		n->pprev = pprev;
		*pprev = n;
		pprev = &n->next;
	}
	*pprev = NULL;
	b->cx = b->cy = 0;
	b->curr = b->scrn_first = b->first;
	fclose(f);
	return 0;
}

void buf_kb_movex(struct buf *b, long offset)
{
	size_t x = b->cx + offset;
	if ((ssize_t)x <= 0)
		x = 0;
	else if (x >= b->curr->size)
		x = b->curr->size - 1;
	b->cx = x;
	buf_show(b);
}

void buf_kb_movey(struct buf *b, long offset)
{
	struct line *n = b->curr;
	if (offset > 0) {
		while (offset && b->cy + 1 < SCRY && n->next) {
			offset--;
			b->cy++;
			n = n->next;
		}
		while (offset-- && n->next && b->scrn_first->next) {
			b->scrn_first = b->scrn_first->next;
			n = n->next;
		}
	} else {
		while (offset && b->cy && n->pprev != &b->first) {
			offset++;
			b->cy--;
			/* Well... we assume offsetof(n, next) is 0 */
			n = (struct line *)n->pprev;
		}
		while (offset++ && n->pprev != &b->first) {
			n = (struct line *)n->pprev;
		}
		if (!b->cy)
			b->scrn_first = n;
	}
	b->curr = n;
	buf_kb_movex(b, 0);
}

void buf_kb_G(struct buf *b)
{
	struct line *n = b->curr;
	while (b->cy + 1 < SCRY && n->next) {
		b->cy++;
		n = n->next;
	}
	while (n->next && b->scrn_first->next) {
		b->scrn_first = b->scrn_first->next;
		n = n->next;
	}
	b->curr = n;
	buf_show(b);
}

void buf_kb_g(struct buf *b)
{
	b->curr = b->scrn_first = b->first;
	b->cy = 0;
	buf_show(b);
}

void buf_kb_H(struct buf *b)
{
	b->curr = b->scrn_first;
	b->cy = 0;
	buf_show(b);
}

void buf_kb_M(struct buf *b)
{
	int flag = 1;
	size_t n = SCRY;
	struct line *cp = b->scrn_first;
	b->curr = cp;
	b->cy = 0;
	while (n-- > 1 && cp->next) {
		cp = cp->next;
		if (flag) {
			b->curr = b->curr->next;
			b->cy++;
		}
		flag = !flag;
	}
	buf_show(b);
}

void buf_kb_L(struct buf *b)
{
	size_t n = SCRY;
	b->curr = b->scrn_first;
	while (n-- > 1 && b->curr->next)
		b->curr = b->curr->next;
	b->cy = SCRY - 1;
	buf_show(b);
}

void buf_kb_i(struct buf *b)
{
	int c;
	struct line *n;
	while ((c = getkb()) != 033) {
		b->curr = line_inschar(b->curr, b->cx, c);
		b->cx++;
		buf_show(b);
	}
	buf_kb_movex(b, -1);
}

void buf_kb_a(struct buf *b)
{
	b->cx++;
	buf_show(b);
	buf_kb_i(b);
}

void buf_kb_x(struct buf *b)
{
	line_delchar(b->curr, b->cx);
	if (b->cx >= b->curr->size) {
		b->cx = b->curr->size - 1;
	}
	buf_show(b);
}

void buf_kb_X(struct buf *b)
{
	if (b->cx <= 0)
		return;
	b->cx--;
	line_delchar(b->curr, b->cx);
	buf_show(b);
}

void buf_kb_locs(struct buf *b, int c)
{
	switch (c) {
	case 'h': return buf_kb_movex(b, -1);
	case 'k': return buf_kb_movey(b, -1);
	case 'j': return buf_kb_movey(b, +1);
	case 'l': return buf_kb_movex(b, +1);
	case 'g': return buf_kb_g(b);
	case 'G': return buf_kb_G(b);
	case 'H': return buf_kb_H(b);
	case 'M': return buf_kb_M(b);
	case 'L': return buf_kb_L(b);
	}
}

void buf_kbcmd(struct buf *b)
{
	int c = getchar();
	switch (c) {
	case 'Z': exit(0);
	case 'i': return buf_kb_i(b);
	case 'a': return buf_kb_a(b);
	case 'x': return buf_kb_x(b);
	case 'X': return buf_kb_X(b);
	default: buf_kb_locs(b, c);
	}
}

int getkb(void)
{
	return getchar();
}

struct termios old_tc, new_tc;

void endscrn(void)
{
	putchar('\n');
	tcsetattr(0, TCSANOW, &old_tc);
}

void initscrn(void)
{
	if (tcgetattr(0, &old_tc) == -1) {
		fprintf(stderr, "vi: input not from terminal, exiting\n");
		exit(1);
	}
	memcpy(&new_tc, &old_tc, sizeof(struct termios));
	new_tc.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &new_tc);
	atexit(endscrn);
	signal(SIGINT, exit);
}

int main(int argc, char **argv)
{
	struct buf b;
	initscrn();
	if (!argv[1]) {
		fprintf(stderr, "usage: vi <file>\n");
		return 1;
	}
	if (buf_load(&b, argv[1]) == -1)
		return 1;
	buf_show(&b);
	while (1)
		buf_kbcmd(&b);
}

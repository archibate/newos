#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$
#/tmp/$$ build/vmlinux.bin > build/vmlinux.bin.hz
/tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct node {
	struct node *left;
	union {
		struct node *right;
		int ch;
	};
	int value;
};

struct node *nt[256];
int tab[256], nts;

void print_n(struct node *n)
{
	if (!n->left) {
		eprintf("%02x:%d", n->ch, n->value);
		return;
	}
	eprintf("[");
	print_n(n->left);
	eprintf(" ");
	print_n(n->right);
	eprintf("]");
}

void print_nt(void)
{
	int i;
	for (i = 0; i < nts; i++) {
		print_n(nt[i]);
		eprintf("\n");
	}
}

void build_nt(void)
{
	int i, m1, m2;
	struct node *n;
	for (i = 0; i < 256; i++) {
		if (tab[i]) {
			nt[nts] = calloc(sizeof(struct node), 1);
			nt[nts]->ch = i;
			nt[nts]->value = tab[i];
			nts++;
		}
	}
	while (nts > 1) {
		//print_nt(); eprintf("============\n");
		m1 = nt[1]->value < nt[0]->value;
		m2 = !m1;
		for (i = 2; i < nts; i++) {
			if (nt[i]->value < nt[m1]->value) {
				m2 = m1;
				m1 = i;
			} else if (nt[i]->value < nt[m2]->value) {
				m2 = i;
			}
		}
		assert(m1 != m2);
		for (i = 0; i < nts; i++) {
			assert(nt[m1]->value <= nt[i]->value);
			assert(i == m1 || nt[m2]->value <= nt[i]->value);
		}
		i = m1 < m2 ? m1 : m2;
		n = calloc(sizeof(struct node), 1);
		n->left = nt[m1];
		n->right = nt[m2];
		n->value = nt[m1]->value + nt[m2]->value;
		nt[i] = n;
		nts--;
		for (i = m1 > m2 ? m1 : m2; i < nts; i++)
			nt[i] = nt[i + 1];
		nt[nts] = NULL;
	}
	print_nt();
}

#ifndef _UNCOMP
int bmp[256], blen[256];

void print_bmp(void)
{
	int i, m;
	for (i = 0; i < 256; i++) {
		if (!blen[i])
			continue;
		eprintf("%02x: ", i);
		for (m = 1 << blen[i] - 1; m; m >>= 1) {
			if (bmp[i] & m)
				eprintf("1");
			else
				eprintf("0");
		}
		eprintf("\n");
	}
}

void build_bmp_n(struct node *n, int val, int bl)
{
	assert(!(val & 1 << 31));
	if (!n->left) {
		bmp[n->ch] = val;
		blen[n->ch] = bl;
		return;
	}
	build_bmp_n(n->left, val << 1, bl + 1);
	build_bmp_n(n->right, val << 1 | 1, bl + 1);
}

void build_bmp(void)
{
	build_bmp_n(nt[0], 0, 0);
	print_bmp();
}

void translate(FILE *fp)
{
	int c, m, k = 1, r = 0;
	while (EOF != (c = getc(fp))) {
		for (m = 1 << blen[c] - 1; m; m >>= 1) {
			if (k & 1 << 8) {
				putc(r, stdout);
				k = 1;
				r = 0;
			}
			if (bmp[c] & m)
				r |= k;
			k <<= 1;
		}
	}
	if (k != 1)
		putc(r, stdout);
}

void write_cthdr(void)
{
	fputs("HZ", stdout);
	if (fwrite(tab, sizeof(tab), 1, stdout) != 1)
		perror("<stdout>");
	fputs("OK", stdout);
}
#endif

int main(int argc, char **argv)
{
	int c, i;
	FILE *fp = stdin;
	if (argv[1]) {
		fp = fopen(argv[1], "r");
		if (!fp)
			perror(argv[1]);
	}

	while (EOF != (c = getc(fp)))
		tab[c]++;
	if (fseek(fp, 0, SEEK_SET) == -1) {
		perror(argv[1] ? argv[1] : "<stdin>");
		return 1;
	}
	write_cthdr();
	build_nt();
	build_bmp();
	translate(fp);

	if (fp != stdin)
		fclose(fp);
	return 0;
}

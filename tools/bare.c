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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

FILE *f;
int over, chr, num;
char buf[233];
fpos_t lab[256];
int mem[65536];
int sp = 65535;

void getchr(void)
{
	chr = getc(f);
}

void savenumpos(void)
{
	fgetpos(f, &lab[num % 256]);
}

void rstrnumpos(void)
{
	fsetpos(f, &lab[num % 256]);
}

void getid(void)
{
	int i = 0;
	while (isspace(chr))
		getchr();
	while (isalnum(chr)) {
		buf[i++] = chr;
		getchr();
	}
	buf[i] = 0;
}

void getnum(void)
{
	num = 0;
	while (isblank(chr))
		getchr();
	while (isdigit(chr)) {
		num = num * 10 + (chr - '0');
		getchr();
	}
}

void skip2nl(void)
{
	while (chr != '\n')
		getchr();
}

void doline(void)
{
	getid();
	if (!strcmp(buf, "ldi")) {
		getnum();
		mem[--sp] = num;
	} else if (!strcmp(buf, "out")) {
		putchar(mem[sp++]);
	} else if (!strcmp(buf, "add")) {
		mem[sp+1] += mem[sp];
		sp++;
	} else if (!strcmp(buf, "sub")) {
		mem[sp+1] -= mem[sp];
		sp++;
	} else if (!strcmp(buf, "mul")) {
		mem[sp+1] *= mem[sp];
		sp++;
	} else if (!strcmp(buf, "dvm")) {
		num = mem[sp+1] / mem[sp];
		mem[sp+1] %= mem[sp];
		mem[sp] = num;
	} else if (!strcmp(buf, "dup")) {
		mem[sp-1] = mem[sp];
		sp--;
	} else if (!strcmp(buf, "pop")) {
		sp++;
	} else if (!strcmp(buf, "swp")) {
		num = mem[sp+1];
		mem[sp+1] = mem[sp];
		mem[sp] = num;
	} else if (!strcmp(buf, "jmp")) {
		getnum();
		rstrnumpos();
	} else if (!strcmp(buf, "bnz")) {
		if (mem[sp]) {
			getnum();
			rstrnumpos();
		}
	} else if (!strcmp(buf, "lab")) {
		getnum();
		savenumpos();
	} else if (!strcmp(buf, "end")) {
		over = 1;
	}
	skip2nl();
}

int runfile(void)
{
	getchr();
	while (!over)
		doline();
}


int main(int argc, char **argv)
{
#ifdef _DEBUG
	argv[1] = "tools/test.ba";
#endif
	if (argv[1]) {
		f = fopen(argv[1], "r");
		if (!f) {
			perror(argv[1]);
			return 1;
		}
	} else {
		f = stdin;
	}
	runfile();
	if (f != stdin)
		fclose(f);
	return 0;
}

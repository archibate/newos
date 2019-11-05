#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -ggdb -gstabs+ -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
#cgdb /tmp/$$ $*
x=$?
rm -f /tmp/$$
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#ifdef _DEBUG
#define debug printf
#else
#define debug(...) do; while (0)
#endif

enum {
	T_EOF = 0,
	T_ID, T_NUM, T_I, T_O, T_OO,
};

int errcnt, token_type;
char token_string[233];

void argv_push(const char *s);
void open_push(const char *path, int fd, int flags);

void error(const char *s)
{
	fprintf(stderr, "error: %s\n", s);
	errcnt++;
}

int good(int c)
{
	return !strchr(" \n\t\r><'\"|&\xff", c);
}

void tokestr(void)
{
	int c, i = 0;
	while (good(c = getc(stdin))) {
		token_string[i++] = c;
	}
	ungetc(c, stdin);
	token_string[i] = 0;
}

void toke(void)
{
	int c;
rep:
	c = getc(stdin);
	if (strchr(" \t\r", c))
		goto rep;
	switch (c) {
	case EOF:
	case '\n':
		token_type = T_EOF;
		break;
	case '<':
		token_type = T_I;
		break;
	case '>':
		c = getc(stdin);
		if (c == '>') {
			token_type = T_OO;
		} else {
			ungetc(c, stdin);
			token_type = T_O;
		}
		break;
	default:
		token_type = T_ID;
		ungetc(c, stdin);
		tokestr();
		break;
	}
}

void factor(void)
{
	if (token_type != T_ID) {
		error("except T_ID");
		token_string[0] = 0;
	}
}

void term(void)
{
	int fd = -1, flags = -1;
rep:
	switch (token_type) {
	case T_I:
		if (fd == -1) fd = 0;
		flags = O_RDONLY;
		break;
	case T_O:
		if (fd == -1) fd = 1;
		flags = O_WRONLY | O_TRUNC | O_CREAT;
		break;
	case T_OO:
		if (fd == -1) fd = 1;
		flags = O_WRONLY | O_CREAT | O_APPEND;
		break;
	case T_ID:
		goto id;
	case T_EOF:
		return;
	default:
		error("except T_ID, '<', '>' or '>>'");
	};
	toke();
	factor();
	open_push(token_string, fd, flags);
	fd = -1;
	flags = -1;
	toke();
	goto rep;
id:
	factor();
	argv_push(token_string);
	toke();
	goto rep;
}

void parse_input(void)
{
	toke();
	term();
}

#define MAX_OPEN 233
struct open_info {
	char *path;
	int fd, flags;
} openv[MAX_OPEN];
int openc;

void open_push(const char *path, int fd, int flags)
{
	debug("open('%s', %d, %d)\n", path, fd, flags);
	if (openc >= MAX_OPEN) {
		error("too much I/O redir");
		return;
	}
	openv[openc].path = strdup(path);
	openv[openc].fd = fd;
	openv[openc].flags = flags;
	openc++;
}

#define MAX_ARGV 233
char *argv[MAX_ARGV + 1];
int argc;

void argv_push(const char *s)
{
	debug("argv('%s')\n", token_string);
	if (argc >= MAX_ARGV) {
		error("too much argv");
		return;
	}
	argv[argc++] = strdup(s);
}

void clear_state(void)
{
	argc = 0;
	openc = 0;
}

void do_opens(void)
{
	for (int i = 0; i < openc; i++) {
		int fd = open(openv[i].path, openv[i].flags, S_IFREG | 0644);
		if (fd == -1) {
			perror(openv[i].path);
			continue;
		}
		close(openv[i].fd);
		dup2(fd, openv[i].fd);
		close(fd);
	}
}

void execute(void)
{
	argv[argc] = NULL;
	if (!argv[0])
		return;

	pid_t pid = fork();
	if (pid == 0) {
		do_opens();
		execvp(argv[0], argv); 
		perror(argv[0]);
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("fork");
		return;
	}
	int stat;
	if (-1 == waitpid(pid, &stat, 0)) {
		perror("waitpid");
		return;
	}
	if (stat) printf("%d ", WEXITSTATUS(stat));
}

int main(void)
{
	while (!feof(stdin)) {
		printf("> ");
		parse_input();
		execute();
		clear_state();
	}
}

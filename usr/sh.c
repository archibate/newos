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
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#ifdef _DEBUG
#define debug(...) eprintf(__VA_ARGS__)
#else
#define debug(...) do {} while (0)
#endif

enum {
	T_EOF = 0,
	T_ID, T_NUM, T_I, T_O, T_OO, T_PIPE,
};

int errcnt, token_type;
char token_string[233];

void argv_push(const char *s);
void open_push(const char *path, int fd, int flags);
void term_push(void);

void error(const char *s)
{
	eprintf("error: %s\n", s);
	errcnt++;
}

void tokestr(void)
{
	int c, i = 0;
	while (1) {
		while (!strchr("\xff \n\t\r><'\"|&\\", c = getc(stdin)))
			token_string[i++] = c;

		if (c == '\\') {
			c = getc(stdin);
			if (c == EOF) {
				error("expect character after `\\`, got EOF");
				break;
			}
			token_string[i++] = c;

		} else if (c == '\'') {
			while (!strchr("\xff'", c = getc(stdin)))
				token_string[i++] = c;
			if (c == EOF) {
				error("expect terminating `'`, got EOF");
				break;
			}

		} else if (c == '"') {
rep_double_quoter:
			while (!strchr("\xff\"\\", c = getc(stdin)))
				token_string[i++] = c;
			if (c == EOF) {
				error("expect terminating `\"`, got EOF");
				break;
			}
			if (c == '\\') {
				c = getc(stdin);
				if (c == EOF) {
					error("expect character after `\\`, got EOF");
					break;
				}
				if (!strchr("\\\"$", c))
					token_string[i++] = '\\';
				token_string[i++] = c;
				goto rep_double_quoter;
			}

		} else break;
	}
	if (c != EOF)
		ungetc(c, stdin);
	token_string[i] = 0;
}

int str_is_digital(const char *s)
{
	for (; *s; s++)
		if (!('0' <= *s && *s <= '9'))
			return 0;
	return 1;
}

void toke(void)
{
	int c;
rep:
	c = getc(stdin);
	if (strchr(" \t\r", c))
		goto rep;
	token_string[0] = 0;
re_switch:
	switch (c) {
	case EOF:
	case '#':
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
			if (c != EOF)
				ungetc(c, stdin);
			token_type = T_O;
		}
		break;
	case '|':
		token_type = T_PIPE;
		break;
	default:
		token_type = T_ID;
		if (c != EOF)
			ungetc(c, stdin);
		tokestr();
		if (!str_is_digital(token_string))
			break;
		c = getc(stdin);
		switch (c) {
		case '<': case '>':
			goto re_switch;
		default:
			if (c != EOF)
				ungetc(c, stdin);
		}
		break;
	}
}

void factor(void)
{
	if (token_type != T_ID) {
		error("expect T_ID");
		token_string[0] = 0;
	}
}

void term(void)
{
	int fd, flags, had = 0;
rep:
	fd = token_string[0] ? atoi(token_string) : -1;
	flags = -1;
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
	default:
		if (!had)
			error("unexpected token term end");
		term_push();
		return;
	};
	toke();
	factor();
	open_push(token_string, fd, flags);
	toke();
	goto rep;
id:
	had = 1;
	factor();
	argv_push(token_string);
	toke();
	goto rep;
}

void expr(void)
{
	term();
	while (token_type == T_PIPE) {
		toke();
		term();
	}
}

void parse_input(void)
{
	toke();
	if (token_type == T_EOF)
		return;
	expr();
	if (token_type != T_EOF)
		error("expect T_EOF token");
}

#define MAX_OPEN 233
#define MAX_ARGV 233
#define MAX_TERM 23

struct open_info {
	char *path;
	int fd, flags;
};

struct term_info {
	struct open_info openv[MAX_OPEN];
	int openc;
	char *argv[MAX_ARGV + 1];
	int argc;
} ts[MAX_TERM];
int tc;

void open_push(const char *path, int fd, int flags)
{
	debug("open('%s', %d, %d)\n", path, fd, flags);
	if (ts[tc].openc >= MAX_OPEN) {
		error("too much I/O redir");
		return;
	}
	ts[tc].openv[ts[tc].openc].path = strdup(path);
	ts[tc].openv[ts[tc].openc].fd = fd;
	ts[tc].openv[ts[tc].openc].flags = flags;
	ts[tc].openc++;
}

void argv_push(const char *s)
{
	debug("argv('%s')\n", token_string);
	if (ts[tc].argc >= MAX_ARGV) {
		error("too much argv");
		return;
	}
	ts[tc].argv[ts[tc].argc++] = strdup(s);
}

void term_push(void)
{
	debug("term()\n");
	tc++;
	ts[tc].argc = 0;
	ts[tc].openc = 0;
}

void clear_state(void)
{
	tc = 0;
	ts[tc].argc = 0;
	ts[tc].openc = 0;
	errcnt = 0;
}

int last_exit_stat;

void wait_for(pid_t pid)
{
	int stat;
	if (waitpid(pid, &stat, 0) < 0) {
		perror("waitpid");
		return;
	}
	last_exit_stat = WEXITSTATUS(stat);

	if (WIFSIGNALED(stat)) {
		stat = WTERMSIG(stat);
		switch (stat) {
		case SIGINT:
			eprintf("Interrupted\n");
			break;
		case SIGABRT:
			eprintf("Aborted\n");
			break;
		case SIGTERM:
			eprintf("Terminated\n");
			break;
		case SIGSEGV:
			eprintf("Segmentation Fault\n");
			break;
		case SIGILL:
			eprintf("Illegal Opcode\n");
			break;
		case SIGFPE:
			eprintf("Floating point exception\n");
			break;
		default:
			eprintf("Terminated on signal %d\n", stat);
		}
	} else if (WIFSTOPPED(stat)) {
		eprintf("Stopped\n");
	}
}

int last_pipe_fd;

int do_opens(int ti)
{
	for (int i = 0; i < ts[ti].openc; i++) {
		int fd = open(ts[ti].openv[i].path, ts[ti].openv[i].flags, S_IFREG | 0644);
		if (fd == -1) {
			perror(ts[ti].openv[i].path);
			return 0;
		}
		close(ts[ti].openv[i].fd);
		dup2(fd, ts[ti].openv[i].fd);
		close(fd);
	}
	return 1;
}

__attribute__((noreturn)) void do_exec(int ti, int i)
{
	if (!do_opens(ti))
		exit(EXIT_FAILURE);
	execvp(ts[ti].argv[i], ts[ti].argv + i); 
	if (errno != ENOENT)
		perror(ts[ti].argv[i]);
	else
		eprintf("%s: command not found\n", ts[ti].argv[i]);
	exit(EXIT_FAILURE);
}

pid_t do_forkexec(int ti)
{
	int fd[2];
	if (ti < tc - 1 && pipe(fd) == -1) {
		perror("pipe");
		return -1;
	}
	pid_t pid = fork();
	if (pid == 0) {
		if (ti < tc - 1) {
			close(fd[0]);
			close(1);
			dup2(fd[1], 1);
			close(fd[1]);
		}
		if (last_pipe_fd != 0) {
			close(0);
			dup2(last_pipe_fd, 0);
			close(last_pipe_fd);
		}
		do_exec(ti, 0);
	} else if (pid < 0) {
		perror("fork");
		return -1;
	}
	if (ti < tc - 1) {
		close(fd[1]);
		last_pipe_fd = fd[0];
	}
	return pid;
}

char pwd[233];

void do_chdir(const char *path)
{
	if (chdir(path) == -1) {
		perror(path);
		last_exit_stat = EXIT_FAILURE;
	} else if (!getcwd(pwd, sizeof(pwd)))
		pwd[0] = 0;
}

pid_t tpids[MAX_TERM];

void execute_ti(int ti)
{
	ts[ti].argv[ts[ti].argc] = NULL;
	if (!ts[ti].argv[0])
		return;
	if (!strcmp(ts[ti].argv[0], "cd")) {
		do_chdir(ts[ti].argv[1] ? ts[ti].argv[1] : "/root");
		return;
	} else if (!strcmp(ts[ti].argv[0], "exit")) {
		exit(ts[ti].argv[1] ? atoi(ts[ti].argv[1]) : last_exit_stat);
	} else if (!strcmp(ts[ti].argv[0], "exec")) {
		do_exec(ti, 1);
	}

	pid_t pid = do_forkexec(ti);
	tpids[ti] = pid;
	if (pid == -1)
		last_exit_stat = 0xff;
}

void execute(void)
{
	last_pipe_fd = 0;
	for (int i = 0; i < tc; i++)
		execute_ti(i);
	for (int i = 0; i < tc; i++)
		if (tpids[i] != -1)
			wait_for(tpids[i]);
}

int main(void)
{
	if (!getcwd(pwd, sizeof(pwd)))
		pwd[0] = 0;
	while (!feof(stdin)) {
		if (last_exit_stat)
		eprintf("%d ", last_exit_stat);
		eprintf("%s # ", pwd);
		parse_input();
		if (!errcnt)
			execute();
		clear_state();
	}
	exit(last_exit_stat);
}

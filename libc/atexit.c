#include <stdlib.h>
#include <stdlib.h>

static struct ate {
	struct ate *next;
	void (*proc)(int, void *);
	void *arg;
} *first;

int atexit(void (*proc)(void))
{
	return on_exit((void *)proc, NULL);
}

int on_exit(void (*proc)(int, void *), void *arg)
{
	struct ate *e;
	e = malloc(sizeof(struct ate));
	if (!e)
		return -1;
	e->next = first;
	e->proc = proc;
	e->arg = arg;
	first = e;
	return 0;
}

void call_on_exit_hooks(int status)
{
	while (first) {
		first->proc(status, first->arg);
		first = first->next;
	}
}

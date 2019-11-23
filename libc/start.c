#include <stdlib.h>
#include <unistd.h>

void __stdio_init_buffers(void);
void call_on_exit_hooks(int status);

__attribute__((noreturn)) void
__crt_startup(
	int (*main)(int, char **, char **),
	int argc, char **argv, char **envp, int envc)
{
	environ = envp;
	__stdio_init_buffers();
	exit(main(argc, argv, envp));
}

void exit(int status)
{
	call_on_exit_hooks(status);
	_exit(status);
}

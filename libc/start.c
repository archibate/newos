#include <stdlib.h>
#include <unistd.h>

void __stdio_init_buffers(void);

__attribute__((noreturn)) void
__crt_startup(
	int (*main)(int argc, char **argv, char **envp),
	int argc, char **argv, char **envp, int envc)
{
	environ = envp;
	__stdio_init_buffers();
	exit(main(argc, argv, envp));
}

void exit(int status)
{
	_exit(status);
}

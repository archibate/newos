#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp);
void __stdio_init_buffers(void);

__attribute__((weak)) int __no_libc_init = 0;

__attribute__((noreturn)) void
_start(int argc, char **argv, char **envp, int envc)
{
	environ = envp;
	if (!__no_libc_init) {
		__stdio_init_buffers();
	}
	exit(main(argc, argv, envp));
}

void exit(int status)
{
	_exit(status);
}

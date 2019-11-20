int main(int argc, char **argv, char **envp);

__attribute__((noreturn)) void
__crt_startup(void *main, int argc, char **argv, char **envp, int envc);

__attribute__((noreturn)) void
_start(int argc, char **argv, char **envp, int envc)
{
	__crt_startup(main, argc, argv, envp, envc);
}

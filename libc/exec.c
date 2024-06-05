#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int execv(const char *path, char *const *argv)
{
	return execve(path, argv, environ);
}

int execvpe(const char *file, char *const *argv, char *const *envp)
{
	int i;
	for (i = 0; file[i]; i++)
		if (file[i] == '/')
			return execv(file, argv);
	const char *prefix = getenv("PATH");
	if (!*prefix) {
		return execve(file, argv, envp);
	}
	do {
		for (i = 0; prefix[i] && prefix[i] != ':'; i++);
		char path[i + 1 + strlen(file)];
		memcpy(path, prefix, i);
		path[i] = '/';
		strcpy(path + i + 1, file);
		execve(path, argv, envp);
		prefix += i + 1;
	} while (prefix[-1]);
	return -1;
}

int execvp(const char *file, char *const *argv)
{
	return execvpe(file, argv, environ);
}

#define EXECL_GETER() \
	va_list ap, pa; \
	va_start(ap, arg); \
	va_start(pa, arg); \
	int argv_size = 1, i = 0; \
	const char *gra = arg; \
	while (gra) { \
		argv_size++; \
		gra = va_arg(pa, const char *); \
	} \
	va_end(pa); \
	char *argv[argv_size]; \
	while (arg) { \
		argv[i++] = (char *)arg; \
		arg = va_arg(ap, const char *); \
	} \
	argv[i] = NULL;

#define EXECL_EXIT() \
	va_end(ap);

int execle(const char *path, const char *arg, ...)
{
	EXECL_GETER();
	char *const *envp = va_arg(ap, char *const *);
	int ret = execve(path, argv, envp);
	EXECL_EXIT();
	return ret;
}

int execl(const char *path, const char *arg, ...)
{
	EXECL_GETER();
	int ret = execv(path, argv);
	EXECL_EXIT();
	return ret;
}

int execlp(const char *file, const char *arg, ...)
{
	EXECL_GETER();
	int ret = execvp(file, argv);
	EXECL_EXIT();
	return ret;
}

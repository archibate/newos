#include <string.h>
#include <malloc.h>

char *strdup(const char *s)
{
	size_t n = strlen(s);
	char *p = malloc(n + 1);
	if (p) strcpy(p, s);
	return p;
}

char *strndup(const char *s, size_t n)
{
	n = strnlen(s, n);
	char *p = malloc(n + 1);
	if (p) strncpy(p, s, n + 1);
	return p;
}

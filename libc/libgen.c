#include <libgen.h>
#include <stddef.h>

char *dirname(char *path)
{
	char *p, *q, *op = NULL;
	for (p = path; *p == '/'; p++);
	for (; *p; p++) {
		if (*p == '/') {
			q = p;
			while (*++p == '/');
			if (*p) op = q;
		}
	}
	if (op)
		*op = 0;
	else if (path[0] == '/')
		path = (char *)"/";
	else
		path = (char *)".";
	return path;
}

char *basename(char *path)
{
	char *p, *q, *bp = path;
	for (p = path; *p; p++) {
		if (*p == '/') {
			q = p;
			while (*++p == '/');
			if (*p) bp = p;
			else *q = 0;
		}
	}
	return bp;
}

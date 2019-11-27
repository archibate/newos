#include <stdio.h>
#include <string.h>
#include <ctype.h>

int chrescape(int c)
{
	switch (c) {
	case '\n': return 'n';
	case '\r': return 'r';
	case '\t': return 't';
	case '\b': return 'b';
	case '\v': return 'v';
	case '\f': return 'f';
	case '\\': return '\\';
	default:   return 0;
	}
}

char *strescape(const char *s)
{
	int c, e, any = 0;
	static char buf[233];
	char *p = buf + 1;
	buf[0] = '\'';
	while ((c = *s++)) {
		if (!isprint(c)) {
			any = 1;
			*p++ = '\\';
			e = chrescape(c);
			if (e) {
				*p++ = e;
			} else {
				sprintf(p, "%#o", c & 0xff);
				p += strlen(p);
			}
		} else {
			*p++ = c;
		}
	}
	if (any) *p++ = '\'';
	*p = 0;
	return any ? buf : buf + 1;
}

#include <stdio.h>
#include <string.h>
#include <ctype.h>

int chrprint(int c)
{
	if (!isprint(c)) {
		c |= '@';
		c &= 0x7f;
		if (c == 0x7f)
			c = '?';
		return c;
	}
	return 0;
}

char *strprint(const char *s)
{
	int c;
	static char buf[233];
	char *p = buf;
	while ((c = *s++)) {
		if (!isprint(c)) {
			*p++ = '^';
			c |= '@';
			c &= 0x7f;
			if (c == 0x7f)
				c = '?';
		}
		*p++ = c;
	}
	*p = 0;
	return buf;
}

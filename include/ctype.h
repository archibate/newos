#ifndef _CTYPE_H
#define _CTYPE_H

static inline int isalpha(int c)
{
	return 'a' <= c && c <= 'z';
}

static inline int isdigit(int c)
{
	return '0' <= c && c <= '9';
}

static inline int isxdigit(int c)
{
	return isdigit(c) ||
		('A' <= c && c <= 'F') ||
		('a' <= c && c <= 'f');
}

static inline int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

static inline int iscntrl(int c)
{
	return 0 <= c && c <= 0x1f;
}

static inline int isgraph(int c)
{
	return 0x21 <= c && c <= 0x7e;
}

static inline int isprint(int c)
{
	return 0x20 <= c && c <= 0x7e;
}

static inline int isascii(int c)
{
	return 0 <= c && c <= 0x7f;
}

static inline int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

static inline int isspace(int c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static inline int isblank(int c)
{
	return c == ' ' || c == '\t';
}

static inline int islower(int c)
{
	return 'a' <= c && c <= 'z';
}

static inline int isupper(int c)
{
	return 'A' <= c && c <= 'Z';
}

static inline int tolower(int c)
{
	if (islower(c))
		return c - 'A' + 'a';
	return c;
}

static inline int toupper(int c)
{
	if (islower(c))
		return c - 'a' + 'A';
	return c;
}

#endif

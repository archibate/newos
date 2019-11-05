#ifndef _BITS_WAIT_H
#define _BITS_WAIT_H 1

#define WNOHANG		1
#define WUNTRACED	2

#define WEXITSTATUS(w)	(((w) >> 8) & 0xff)
#define _MAKE_WSTAT(status) \
	(((status) & 0xff) << 8)

#endif

#ifndef _BITS_TERMIOS_H
#define _BITS_TERMIOS_H 1

// bits for c_iflag:
#define ICRNL	1
#define INLCR	2
#define IGNCR	3
#define IMAXBEL	4

// bits for c_oflag:
#define OCRNL	1
#define ONLCR	2
#define ONOCR	3

// bits for c_lflag:
#define ECHO	1
#define ICANON	2
#define ISIG	4
#define ECHONL	8
#define ECHOE	16
#define ECHOK	32

// values for c_cc:
#define VEOL		0
#define VEOF		1
#define VINTR		2
#define VERASE		3
#define VEOL2		4
#define VBELL		5
#define NCCS		6

#define DEFAULT_TERMIOS { \
	.c_iflag = IMAXBEL, \
	.c_lflag = ECHO | ICANON | ISIG | ECHOE | ECHOK, \
	.c_oflag = 0, \
	.c_cflag = 0, \
	.c_cc = { \
		[VEOL] = '\n', \
		[VEOL2] = '\r', \
		[VERASE] = '\b', \
		[VINTR] = 003, /* Ctrl-C */ \
		[VEOF] = 004, /* Ctrl-D */ \
		[VBELL] = '\a', \
	}, \
}

typedef int tcflag_t, cc_t;

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_lflag;
	tcflag_t c_cflag;
	cc_t c_cc[NCCS];
};

// optop for tcsetattr:
#define TCSANOW		1

#endif

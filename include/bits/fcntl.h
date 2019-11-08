#ifndef _BITS_FCNTL_H
#define _BITS_FCNTL_H 1

#define O_RDONLY	1
#define O_WRONLY	2
#define O_RDWR		3
#define O_ACCMODE	3
#define	O_CREAT		4
#define	O_EXCL		8
#define	O_TRUNC		16
#define	O_APPEND	32
#define	O_DIRECTORY	64
#define	O_CLOEXEC	128
#define	O_NOFOLLOW	256

#define F_DUPFD	0
#define F_GETFD	1
#define F_SETFD	2
#define F_GETFL	3
#define F_DUPFD_CLOEXEC	5

#define FD_CLOEXEC	1

#define AT_FDCWD	(-2)

#endif

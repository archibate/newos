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
#define	O_PATH		512
#define	O_SYMLINK	1024
#define	O_NONBLOCK	2048
#define _O_FL_EDIABLE	(O_ACCMODE | O_NONBLOCK)

#define F_DUPFD		0
#define F_GETFD		1
#define F_SETFD		2
#define F_GETFL		3
#define F_SETFL		4
#define F_DUPFD_CLOEXEC	5

#define FD_CLOEXEC	1

#define AT_FDCWD	(-2)

#define AT_EMPTY_PATH		1
#define AT_SYMLINK_NOFOLLOW	2
#define AT_SYMLINK_FOLLOW	2
#define AT_REMOVEDIR		4
#define AT_EACCESS		4

#endif

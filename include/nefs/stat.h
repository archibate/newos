#ifndef _KIP_STAT_H
#define _KIP_STAT_H 1

/* Encoding of the file mode.  */

#define S_IFMT		0170000	/* These bits determine file type.  */

/* File types.  */
#define S_IFDIR		0040000	/* Directory.  */
#define S_IFCHR		0020000	/* Character device.  */
#define S_IFBLK		0060000	/* Block device.  */
#define S_IFREG		0100000	/* Regular file.  */
#define S_IFIFO		0010000	/* FIFO.  */
#define S_IFLNK		0120000	/* Symbolic link.  */
#define S_IFSOCK	0140000	/* Socket.  */

#define S_ISDIR(m)	(S_IFDIR == ((m) & S_IFMT))
#define S_ISCHR(m)	(S_IFCHR == ((m) & S_IFMT))
#define S_ISBLK(m)	(S_IFBLK == ((m) & S_IFMT))
#define S_ISREG(m)	(S_IFREG == ((m) & S_IFMT))
#define S_ISFIFO(m)	(S_IFIFO == ((m) & S_IFMT))
#define S_ISLNK(m)	(S_IFLNK == ((m) & S_IFMT))
#define S_ISSOCK(m)	(S_IFSOCK == ((m) & S_IFMT))

/* POSIX.1b objects.  Note that these macros always evaluate to zero.  But
   they do it by enforcing the correct use of the macros.  */

/* Protection bits.  */

#define S_ISUID	04000	/* Set user ID on execution.  */
#define S_ISGID	02000	/* Set group ID on execution.  */
#define S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define S_IRWXU	0700
#define S_IRUSR	0400	/* Read by owner.  */
#define S_IWUSR	0200	/* Write by owner.  */
#define S_IXUSR	0100	/* Execute by owner.  */
#define S_IRWXG	070
#define S_IRGRP	040	/* Read by group member.  */
#define S_IWGRP	020	/* Write by group member.  */
#define S_IXGRP	010	/* Execute by group member.  */
#define S_IRWXO	07
#define S_IROTH	04	/* Read by other.  */
#define S_IWOTH	02	/* Write by other.  */
#define S_IXOTH	01	/* Execute by other.  */

#endif

#ifndef _KIP_NEFS_H
#define _KIP_NEFS_H 1

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


#define NEFS_NR_DIRECT	11
#define NEFS_MAGIC	0x5346654e // ASCII "NeFS"
#define NEFS_ROOT_INO	1
#define NEFS_NAME_MAX	27
#define NEFS_INODE_SIZE		sizeof(struct nefs_inode)
#define NEFS_DIR_ENTRY_SIZE	sizeof(struct nefs_dir_entry)

#include <stdint.h>
typedef uint32_t nefs_blkno_t, nefs_zone_t;
typedef uint32_t nefs_blkcnt_t;
typedef uint32_t nefs_blksize_t;
typedef uint32_t nefs_size_t;
typedef uint32_t nefs_nlink_t;
typedef uint32_t nefs_mode_t;
typedef uint32_t nefs_time_t;
typedef uint32_t nefs_blksize_log2_t;
typedef uint32_t nefs_ino_t;
typedef uint32_t nefs_word_t;
typedef uint16_t nefs_half_t;
typedef uint8_t nefs_byte_t;
typedef int8_t nefs_char_t;

struct nefs_super_block
{
	nefs_word_t s_magic;
	nefs_size_t s_super_len;
	nefs_blksize_log2_t s_blksize_log2;
	nefs_blkno_t s_imap_begblk;
	nefs_blkno_t s_zmap_begblk;
	nefs_blkno_t s_itab_begblk;
	nefs_blkno_t s_data_begblk;
	nefs_blkcnt_t s_imap_blknr;
	nefs_blkcnt_t s_zmap_blknr;
	nefs_blkcnt_t s_itab_blknr;
	nefs_blkcnt_t s_data_blknr;
};

struct nefs_inode
{
	nefs_mode_t i_nefs_mode;
	nefs_size_t i_nefs_size;
	nefs_nlink_t i_nefs_nlink;
	nefs_zone_t i_nefs_zone[NEFS_NR_DIRECT];
	nefs_zone_t i_nefs_s_zone;
	nefs_time_t i_nefs_atime;
};

struct nefs_dir_entry
{
	nefs_ino_t d_ino;
	nefs_char_t d_name[NEFS_NAME_MAX + 1];
};

#endif

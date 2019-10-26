#ifndef _KERN_NEFS_H
#define _KERN_NEFS_H 1

#define NEFS_NR_DIRECT	11
#define NEFS_MAGIC	0x5346454e // ASCII "NEFS"
#define NEFS_ROOT_INO	1
#define NEFS_INODE_SIZE	sizeof(struct nefs_inode)

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
typedef uint8_t nefs_byte_t;
typedef int8_t nefs_char_t;

struct nefs_super_block
{
	uint32_t s_magic;
	nefs_size_t s_super_len;
	nefs_blksize_log2_t s_blksize_log2;
	nefs_blkno_t s_imap_begin_blk;
	nefs_blkno_t s_zmap_begin_blk;
	nefs_blkno_t s_itab_begin_blk;
	nefs_blkno_t s_data_begin_blk;
	nefs_blkcnt_t s_imap_blknr;
	nefs_blkcnt_t s_zmap_blknr;
	nefs_blkcnt_t s_itab_blknr;
	nefs_blkcnt_t s_data_blknr;
};

struct nefs_inode
{
	nefs_mode_t i_mode;
	nefs_size_t i_size;
	nefs_nlink_t i_nlink;
	nefs_zone_t i_zone[NEFS_NR_DIRECT];
	nefs_zone_t i_s_zone;
	nefs_time_t i_atime;
};

#endif

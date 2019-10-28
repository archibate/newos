#if 0 // {{{
true /*
set -e
chmod +x $0
gcc -D_ARGV0=\"$0\" $0 -o /tmp/$$
/tmp/$$ $*
exit
true */
#endif
#ifndef _ARGV0
#define _ARGV0 (argv[0])
#endif // }}}
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <strings.h>
#include "../include/kern/nefs.h"

#define BSIZE 1024

FILE *fp;
struct nefs_super_block sb;

void touch_seek(off_t pos)
{
	fseek(fp, 0, SEEK_END);
	int n = pos - ftell(fp);
	while (n-- > 0)
		fputc(0, fp);
	fseek(fp, pos, SEEK_SET);
	assert(ftell(fp) == pos);
}

void fillzero(size_t size)
{
	while (size-- > 0) {
		fputc(0, fp);
	}
}

int alloc_zone(void)
{
	fseek(fp, (sb.s_zmap_begblk - 1) * BSIZE, SEEK_SET);
	for (int i = 0; i < sb.s_zmap_blknr * BSIZE; i += sizeof(unsigned)) {
		unsigned r;
		fread(&r, sizeof(r), 1, fp);
		int k = ffs(~r);
		if (k--) {
			fseek(fp, -sizeof(r), SEEK_CUR);
			r |= 1 << k;
			fwrite(&r, sizeof(r), 1, fp);
			return i * 8 + k;
		}
	}
	eprintf("failed to alloc_zone\n");
	assert(0);
}

int alloc_inode(void)
{
	fseek(fp, (sb.s_imap_begblk - 1) * BSIZE, SEEK_SET);
	for (int i = 0; i < sb.s_imap_blknr * BSIZE; i += sizeof(unsigned)) {
		unsigned r;
		fread(&r, sizeof(r), 1, fp);
		int k = ffs(~r);
		if (k--) {
			fseek(fp, -sizeof(r), SEEK_CUR);
			r |= 1 << k;
			fwrite(&r, sizeof(r), 1, fp);
			return i * 8 + k;
		}
	}
	eprintf("failed to alloc_inode\n");
	assert(0);
}

void update_inode(int ino, struct nefs_inode *ip)
{
	char c = 0;
	fseek(fp, (sb.s_itab_begblk - 1) * BSIZE + ino * sizeof(*ip), SEEK_SET);
	fwrite(ip, sizeof(*ip), 1, fp);
}

void increase_i_nlink(int ino)
{
	nefs_nlink_t nlink;
	fseek(fp, (sb.s_itab_begblk - 1) * BSIZE
			+ ino * sizeof(struct nefs_inode)
			+ offsetof(struct nefs_inode, i_nefs_nlink), SEEK_SET);
	fread(&nlink, sizeof(nlink), 1, fp);
	nlink++;
	fseek(fp, -sizeof(nlink), SEEK_CUR);
	fwrite(&nlink, sizeof(nlink), 1, fp);
}

int write_zone(int z, FILE *f)
{
	char buf[BSIZE];
	size_t size = fread(buf, 1, BSIZE, f);
	fseek(fp, (sb.s_data_begblk + z - 1) * BSIZE, SEEK_SET);
	fwrite(buf, size, 1, fp);
	return size == BSIZE;
}

void dir_write_entry(FILE *f, const char *name, int ino)
{
	struct nefs_dir_entry de;
	de.d_ino = ino;
	strncpy(de.d_name, name, NEFS_NAME_MAX);
	de.d_name[NEFS_NAME_MAX] = 0;
	fwrite(&de, sizeof(de), 1, f);
	increase_i_nlink(ino);
}

int set_inode(int ino, FILE *f, int mode)
{
	struct nefs_inode inode;
	memset(&inode, 0, sizeof(inode));
	rewind(f);
	for (int i = 0; i < NEFS_NR_DIRECT; i++) {
		if (feof(f))
			goto eof;
		int z = alloc_zone();
		inode.i_nefs_zone[i] = z;
		if (!write_zone(z, f))
			goto eof;
	}
	int s_zone_buf[BSIZE / sizeof(int)];
	memset(s_zone_buf, 0, BSIZE);
	for (int i = 0; i < BSIZE / sizeof(int); i++) {
		if (feof(f))
			goto s_eof;
		int z = alloc_zone();
		s_zone_buf[i] = z;
		if (!write_zone(z, f))
			goto s_eof;
	}
	eprintf("WARNING: file too big, truncated to %d KB\n", ftell(f) / 1024);
s_eof:
	inode.i_nefs_s_zone = alloc_zone();
	fseek(fp, (sb.s_data_begblk + inode.i_nefs_s_zone - 1) * BSIZE, SEEK_SET);
	fwrite(s_zone_buf, BSIZE, 1, fp);
eof:
	inode.i_nefs_size = ftell(f);
	inode.i_nefs_mode = mode;
	update_inode(ino, &inode);
	return ino;
}

void parse_file_list(int dir, int parent_dir, FILE *fl)
{
	FILE *dir_tmp = tmpfile();

	dir_write_entry(dir_tmp, ".", dir);
	dir_write_entry(dir_tmp, "..", parent_dir);
	while (1) {
		char buf[512];
		if (!fgets(buf, sizeof(buf), fl))
			break;
		int ino = alloc_inode();
		char *destname = strtok(buf, " \t\r\n");
		if (!*destname || !strcmp(destname, "}"))
			break;
		/*if (destname[0] == '@') {
			i_mode = strtol(destname, 8, NULL);
			destname = strtok(buf, " \t\r\n");
		}*/
		char *srcpath = strtok(NULL, " \t\r\n");
		if (!strcmp(srcpath, "{")) {
			parse_file_list(ino, dir, fl);
		} else {
			FILE *sf = fopen(srcpath, "r");
			if (!sf) {
				perror(srcpath);
				sf = tmpfile();
			}
			set_inode(ino, sf, 0644 | S_IFREG);
			fclose(sf);
		}
		dir_write_entry(dir_tmp, destname, ino);
	}

	set_inode(dir, dir_tmp, 0755 | S_IFDIR);
	fclose(dir_tmp);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
usage:
		eprintf("Usage: %s <image> [-L volume-label] [-f file-list]\n"
			"[-i inodes] [-b blocks] [-r reserved-blocks]\n", _ARGV0);
		return EXIT_FAILURE;
	}
	const char *vol_label = "NeFS";
	const char *file_list = "tools/file_list.txt";
	int reserved_blocks = 0;
	int inodes = BSIZE * 8;
	int blocks = BSIZE * 8;
	int ch;
	while (-1 != (ch = getopt(argc, argv, "f:L:i:b:r:"))) {
		switch (ch) {
		case 'f': file_list = optarg; break;
		case 'L': vol_label = optarg; break;
		case 'i': inodes = atoi(optarg); break;
		case 'b': blocks = atoi(optarg); break;
		case 'r': reserved_blocks = atoi(optarg); break;
		default: goto usage;
		}
	}
	fp = fopen(argv[optind], "r+");
	if (!fp) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}
	printf("%s: %d inodes, %d blocks\n", vol_label, inodes, blocks);
	memset(&sb, 0, sizeof(sb));
	switch (0) { case 0: case BSIZE % sizeof(struct nefs_inode) == 0:; }
	sb.s_super_len = sizeof(sb);
	sb.s_itab_blknr = inodes * sizeof(struct nefs_inode) / BSIZE;
	sb.s_imap_blknr = inodes / BSIZE / 8;
	sb.s_zmap_blknr = blocks / BSIZE / 8;
	sb.s_data_blknr = blocks;
	sb.s_magic = NEFS_MAGIC;
	sb.s_blksize_log2 = 10;
	sb.s_imap_begblk = 3 + reserved_blocks;
	sb.s_zmap_begblk = sb.s_imap_begblk + sb.s_imap_blknr;
	sb.s_itab_begblk = sb.s_zmap_begblk + sb.s_zmap_blknr;
	sb.s_data_begblk = sb.s_itab_begblk + sb.s_itab_blknr;
	touch_seek(1 * BSIZE);
	fwrite(&sb, sizeof(sb), 1, fp);
	fwrite(vol_label, strnlen(vol_label, BSIZE - sb.s_super_len), 1, fp);
	touch_seek((sb.s_imap_begblk - 1) * BSIZE);
	fillzero((sb.s_imap_blknr + sb.s_zmap_blknr
		+ sb.s_itab_blknr + blocks) * BSIZE);
	assert(alloc_inode() == 0);
	assert(alloc_zone() == 0);

	FILE *fl = fopen(file_list, "r");
	if (!fl) {
		perror(file_list);
		exit(0);
	}
	assert(alloc_inode() == NEFS_ROOT_INO);
	parse_file_list(NEFS_ROOT_INO, NEFS_ROOT_INO, fl);
	fclose(fl);

	return EXIT_SUCCESS;
}

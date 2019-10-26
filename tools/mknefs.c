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
	fseek(fp, (sb.s_zmap_begin_blk - 1) * BSIZE, SEEK_SET);
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
	fseek(fp, (sb.s_imap_begin_blk - 1) * BSIZE, SEEK_SET);
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
	fseek(fp, (sb.s_itab_begin_blk - 1) * BSIZE + ino * sizeof(*ip), SEEK_SET);
	fwrite(ip, sizeof(*ip), 1, fp);
}

int write_zone(int z, FILE *f)
{
	char buf[BSIZE];
	size_t size = fread(buf, 1, BSIZE, f);
	fseek(fp, (sb.s_data_begin_blk + z - 1) * BSIZE, SEEK_SET);
	fwrite(buf, size, 1, fp);
	return size == BSIZE;
}

int add_inode(const char *file)
{
	FILE *f = fopen(file, "r");
	if (!f) {
		perror(file);
		return 0;
	}
	int ino = alloc_inode();
	struct nefs_inode inode;
	memset(&inode, 0, sizeof(inode));
	for (int i = 0; i < NEFS_NR_DIRECT; i++) {
		if (feof(f))
			goto eof;
		int z = alloc_zone();
		inode.zone[i] = z;
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
	inode.s_zone = alloc_zone();
	fseek(fp, (sb.s_data_begin_blk + inode.s_zone - 1) * BSIZE, SEEK_SET);
	fwrite(s_zone_buf, BSIZE, 1, fp);
eof:
	inode.size = ftell(f);
	update_inode(ino, &inode);
	fclose(f);
	printf("%6d %s\n", ino, file);
	return ino;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
usage:
		eprintf("Usage: %s <image> [-L volume-label]\n"
			"[-i inodes] [-b blocks] [-r reserved-blocks]\n"
			"[copy-in-file-list]\n", _ARGV0);
		return EXIT_FAILURE;
	}
	const char *vol_label = "NeFS";
	int reserved_blocks = 0;
	int inodes = BSIZE * 8;
	int blocks = BSIZE * 8;
	int ch;
	while (-1 != (ch = getopt(argc, argv, "L:i:b:r:"))) {
		switch (ch) {
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
	sb.s_imap_begin_blk = 3 + reserved_blocks;
	sb.s_zmap_begin_blk = sb.s_imap_begin_blk + sb.s_imap_blknr;
	sb.s_itab_begin_blk = sb.s_zmap_begin_blk + sb.s_zmap_blknr;
	sb.s_data_begin_blk = sb.s_itab_begin_blk + sb.s_itab_blknr;
	touch_seek(1 * BSIZE);
	fwrite(&sb, sizeof(sb), 1, fp);
	fwrite(vol_label, strnlen(vol_label, BSIZE - sb.s_super_len), 1, fp);
	touch_seek((sb.s_imap_begin_blk - 1) * BSIZE);
	fillzero((sb.s_imap_blknr + sb.s_zmap_blknr
		+ sb.s_itab_blknr + blocks) * BSIZE);
	assert(alloc_inode() == 0);
	assert(alloc_zone() == 0);
	add_inode("tools/aa.txt");
	fclose(fp);
	fp = NULL;
	return EXIT_SUCCESS;
}

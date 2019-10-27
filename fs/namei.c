#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <string.h>

static int match(struct dir_entry *de, const char *name, size_t namelen)
{
	return strlen(de->d_name) == namelen &&
		!memcmp(de->d_name, name, namelen);
}

static int dir_find_entry(struct inode *dir, struct dir_entry *de,
		const char *name, size_t namelen)
{
	for (size_t pos = 0; pos + NEFS_DIR_ENTRY_SIZE <= dir->i_size;
			pos += NEFS_DIR_ENTRY_SIZE) {
		iread(dir, pos, de, NEFS_DIR_ENTRY_SIZE);
		if (de->d_ino != 0) { // if this entry is not none
			if (match(de, name, namelen)) {
				return pos / NEFS_DIR_ENTRY_SIZE;
			}
		}
	}
	return -1;
}

int dir_read_entry(struct inode *dir, struct dir_entry *de, int i)
{
	if ((i + 1) * NEFS_DIR_ENTRY_SIZE > dir->i_size)
		return -1;
	iread(dir, i * NEFS_DIR_ENTRY_SIZE, de, NEFS_DIR_ENTRY_SIZE);
	return de->d_ino != 0;
}

static int dir_del_entry(struct inode *dir, struct dir_entry *de, int i)
{
	if ((i + 1) * NEFS_DIR_ENTRY_SIZE > dir->i_size)
		return -1;
	iread(dir, i * NEFS_DIR_ENTRY_SIZE, de, NEFS_DIR_ENTRY_SIZE);
	if (de->d_ino == 0)
		return 0;
	return 1;
}

struct inode *namei(const char *path)
{
	size_t namelen;
	struct dir_entry de;
	struct inode *ip;
	if (*path == '/')
		ip = idup(current->root);
	else if (*path != 0)
		ip = idup(current->cwd);
	else
		return NULL;
	for (;; path += namelen) {
		while (*path == '/')
			path++;
		if (*path == 0)
			break;
		namelen = strchrnul(path, '/') - path;
		if (path[0] == '.' && (namelen == 1 || ip == current->root &&
					namelen == 2 && path[1] == '.'))
			continue;
		if (-1 == dir_find_entry(ip, &de, path, namelen)) {
			iput(ip);
			return NULL;
		}
		iput(ip);
		ip = iget(de.d_ino);
	}
	return ip;
}

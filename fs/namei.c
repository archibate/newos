#include <kern/fs.h>
#include <kern/kernel.h>
#include <string.h>

int match(struct nefs_dir_entry *de, const char *name, size_t namelen)
{
	return strlen(de->d_name) == namelen &&
		!memcmp(de->d_name, name, namelen);
}

static int find_entry(struct inode *dir, struct nefs_dir_entry *de,
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

struct inode *namei(const char *path)
{
	size_t namelen;
	const char *p;
	struct nefs_dir_entry de;
	struct inode *ip = iget(NEFS_ROOT_INO);
	while (1) {
		while (*path == '/')
			path++;
		if (*path == 0)
			break;
		p = strchrnul(path, '/');
		if (-1 == find_entry(ip, &de, path, p - path))
			return NULL;
		iput(ip);
		ip = iget(de.d_ino);
		path = p;
	}
	return ip;
}

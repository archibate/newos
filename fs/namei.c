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
		if (de->d_ino != 0) // if this entry is not none
			if (match(de, name, namelen))
				return pos / NEFS_DIR_ENTRY_SIZE;
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

// assumes that dir_find_entry returned -1, add directly
static void dir_add_entry(struct inode *dir, struct inode *ip,
		const char *name, size_t namelen)
{
	struct dir_entry de;
	ip->i_nlink++;
	iupdate(ip);
	de.d_ino = ip->i_ino;
	if (namelen > NEFS_NAME_MAX)
		namelen = NEFS_NAME_MAX;
	memcpy(de.d_name, name, namelen);
	de.d_name[namelen] = 0;
	dir->i_size -= dir->i_size % NEFS_DIR_ENTRY_SIZE;
	iwrite(dir, dir->i_size, &de, NEFS_DIR_ENTRY_SIZE);
}

static int dir_del_entry(struct inode *dir, int i)
{
	nefs_ino_t ino;
	static const char zero[NEFS_DIR_ENTRY_SIZE];
	if ((i + 1) * NEFS_DIR_ENTRY_SIZE > dir->i_size)
		return -1;
	iread(dir, i * NEFS_DIR_ENTRY_SIZE, &ino, sizeof(ino));
	if (ino != 0) {
		struct inode *ip = iget(ino);
		if (!ip) {
			printk("ERROR: de inode %d not exist", ino);
		} else {
			if (ip->i_nlink <= 0)
				printk("ERROR: de inode %d nlink <= 0", ino);
			else
				ip->i_nlink--;
			iupdate(ip);
			iput(ip);
		}
		iwrite(dir, i * NEFS_DIR_ENTRY_SIZE, &zero, sizeof(zero));
	}
	return ino;
}

struct inode *_namei(const char **ppath, struct inode **pip)
{
	size_t namelen;
	struct dir_entry de;
	struct inode *ip;
	const char *path = *ppath;
	*pip = NULL;
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
		if (*pip) iput(*pip);
		*pip = ip;
		if (-1 == dir_find_entry(ip, &de, path, namelen)) {
			*ppath = path;
			return NULL;
		}
		ip = iget(de.d_ino);
	}
	*ppath = path;
	return ip;
}

struct inode *creati(const char *path, int excl)
{
	char *p;
	size_t namelen;
	struct inode *pip, *ip;
	ip = _namei(&path, &pip);
	if (ip != NULL) {
		if (pip) iput(pip);
		if (excl) {
			iput(ip);
			return NULL;
		}
		return ip;
	}
	p = strchrnul(path, '/');
	namelen = p - path;
	while (*p == '/')
		p++;
	if (*p) // no inner directory, /path/_to_/file
		return NULL;
	ip = create_inode(pip);
	dir_add_entry(pip, ip, path, namelen);
	iput(pip);
	return ip;
}

struct inode *namei(const char *path)
{
	struct inode *pip, *ip;
	ip = _namei(&path, &pip);
	if (pip) iput(pip);
	return ip;
}

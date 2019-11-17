#include <kern/fs.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <string.h>
#include <errno.h>

static int match(struct dir_entry *de, const char *name, size_t namelen)
{
	return strlen(de->d_name) == namelen &&
		!memcmp(de->d_name, name, namelen);
}

static int dir_find_entry(struct inode *dir, struct dir_entry *de,
		const char *name, size_t namelen)
{
	if (!S_ISDIR(dir->i_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (iaccess(dir, X_OK, 0) == -1)
		return -1;
	for (size_t pos = 0; pos + NEFS_DIR_ENTRY_SIZE <= dir->i_size;
			pos += NEFS_DIR_ENTRY_SIZE) {
		if (rw_inode(READ, dir, pos, de, NEFS_DIR_ENTRY_SIZE)
			!= NEFS_DIR_ENTRY_SIZE)
			return -1;
		if (de->d_ino != 0) // if this entry is not none
			if (match(de, name, namelen))
				return pos / NEFS_DIR_ENTRY_SIZE;
	}
	errno = ENOENT;
	return -1;
}

int dir_read_entry(struct inode *dir, struct dir_entry *de, int i)
{
	if (!S_ISDIR(dir->i_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (iaccess(dir, R_OK, 0) == -1)
		return -1;
	if ((unsigned)(i + 1) * NEFS_DIR_ENTRY_SIZE > dir->i_size) {
		errno = EINVAL;
		return -1;
	}
	if (iread(dir, i * NEFS_DIR_ENTRY_SIZE, de, NEFS_DIR_ENTRY_SIZE)
			!= NEFS_DIR_ENTRY_SIZE)
		return -1;
	return de->d_ino != 0;
}

// assumes that dir_find_entry check returned -1, add directly
static int dir_add_entry(struct inode *dir, struct inode *ip,
		const char *name, size_t namelen)
{
	struct dir_entry de;
	de.d_ino = ip->i_ino;
	if (namelen > NEFS_NAME_MAX)
		namelen = NEFS_NAME_MAX;
	memcpy(de.d_name, name, namelen);
	de.d_name[namelen] = 0;
	dir->i_size -= dir->i_size % NEFS_DIR_ENTRY_SIZE;
	if (rw_inode(WRITE, dir, dir->i_size, &de, NEFS_DIR_ENTRY_SIZE)
		!= NEFS_DIR_ENTRY_SIZE) // NOTE here, no perm check performed
		return -1;
	ip->i_nlink++;
	iupdate(ip);
	return 0;
}

static int dir_check_if_empty(struct inode *dir)
{
	struct dir_entry de;
	rw_inode(READ, dir, 0, &de, NEFS_DIR_ENTRY_SIZE);
	if (!de.d_ino || !!strcmp(de.d_name, ".")) {
		printk("ERROR: BAD dir . entry");
	}
	rw_inode(READ, dir, NEFS_DIR_ENTRY_SIZE, &de, NEFS_DIR_ENTRY_SIZE);
	if (!de.d_ino || !!strcmp(de.d_name, "..")) {
		printk("ERROR: BAD dir .. entry");
	}
	for (size_t i = 2; i < dir->i_size / NEFS_DIR_ENTRY_SIZE; i++) {
		rw_inode(READ, dir, i * NEFS_DIR_ENTRY_SIZE, &de, NEFS_DIR_ENTRY_SIZE);
		if (de.d_ino)
			return 0;
	}
	return 1;
}

static int dir_del_entry(struct inode *dir, int i, int rmdir);
static int check_rmdir_inode(struct inode *ip, int rmdir)
{
	if (rmdir == 233)
		return 0;
	if (rmdir && !S_ISDIR(ip->i_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (!rmdir && S_ISDIR(ip->i_mode)) {
		errno = EISDIR;
		return -1;
	}
	if (rmdir) {
		if (!dir_check_if_empty(ip)) {
			errno = ENOTEMPTY;
			return -1;
		}
		if (ip->i_nlink != 2) {
			printk("ERROR: bad nlinks %d for empty dir", ip->i_nlink);
			errno = ENOTEMPTY;
			return -1;
		}
		if (dir_del_entry(ip, 0, 233) == -1) {
			printk("ERROR: cannot delete . entry");
			return -1;
		}
		if (dir_del_entry(ip, 1, 233) == -1) {
			printk("ERROR: cannot delete .. entry");
			return -1;
		}
		if (ip->i_nlink != 1) {
			printk("WARNING: bad nlinks %d after del .", ip->i_nlink);
			ip->i_nlink = 1;
		}
	}
	return 0;
}

static int dir_del_entry(struct inode *dir, int i, int rmdir)
{
	if (!S_ISDIR(dir->i_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (rmdir != 233 && iaccess(dir, R_OK | W_OK, 0) == -1)
		return -1;
	if (i == 0 && rmdir != 233) {
		errno = EINVAL;
		return -1;
	}
	if (i == 1 && rmdir != 233) {
		errno = ENOTEMPTY;
		return -1;
	}
	if ((unsigned)(i + 1) * NEFS_DIR_ENTRY_SIZE > dir->i_size) {
		errno = EINVAL;
		return -1;
	}
	nefs_ino_t ino;
	if (rw_inode(READ, dir, i * NEFS_DIR_ENTRY_SIZE, &ino, sizeof(ino)) != sizeof(ino))
		return -1;
	if (ino == 0) {
		errno = ENOENT;
		return -1;
	}
	struct inode *ip = iget(dir->i_dev, ino);
	if (!ip) {
		printk("ERROR: de inode %d not exist", ino);
	} else {
		if (check_rmdir_inode(ip, rmdir) == -1) {
			iput(ip);
			return -1;
		}
		if (ip->i_nlink <= 0)
			printk("ERROR: de inode %d nlink <= 0", ino);
		else
			ip->i_nlink--;
		iupdate(ip);
		iput(ip);
	}
	static char zero[NEFS_DIR_ENTRY_SIZE];
	rw_inode(WRITE, dir, i * NEFS_DIR_ENTRY_SIZE, &zero, sizeof(zero));
	return ino;
}

static int ks_follow = SYMLOOP_MAX, ks_target_nofollow;

void follow_policy_enter(int o_nofollow, int o_symlink)
{
	ks_follow = o_nofollow ? 0 : SYMLOOP_MAX;
	ks_target_nofollow = o_symlink;
}

void follow_policy_leave(void)
{
	ks_follow = SYMLOOP_MAX;
	ks_target_nofollow = 0;
}

static struct inode *ifollow(struct inode *ip, struct inode *pip)
{
	if (!S_ISLNK(ip->i_mode))
		return ip;
	if (ks_follow == 0) {
		errno = ELOOP;
		iput(ip);
		return NULL;
	} else if (ks_follow < 0) {
		return ip;
	}
	if (ip->i_size > PATH_MAX) {
		errno = ENAMETOOLONG;
		iput(ip);
		return NULL;
	}
	char buf[ip->i_size];
	if (iread(ip, 0, buf, ip->i_size) != ip->i_size) {
		iput(ip);
		return NULL;
	}
	iput(ip);
	struct inode *old_cwd = current->cwd;
	current->cwd = pip;
	--ks_follow;
	ip = namei(buf);
	++ks_follow;
	current->cwd = old_cwd;
	return ip;
}

struct inode *_namei(const char **ppath, struct inode **pip,
		const char **ppath2)
{
	size_t namelen;
	struct dir_entry de;
	struct inode *ip;
	const char *path = *ppath;
	int tmp;
	*pip = NULL;
	*ppath2 = path;
	if (*path == '/')
		ip = idup(current->root);
	else if (*path != 0)
		ip = idup(current->cwd);
	else {
		errno = EINVAL;
		return NULL;
	}
	for (;; path += namelen) {
		tmp = *path;
		while (*path == '/')
			path++;
		if (*path == 0) {
			if (tmp == '/' && !S_ISDIR(ip->i_mode)) {
				errno = ENOTDIR;
				return NULL;
			}
			break;
		}
		if (*pip) iput(*pip);
		*pip = ip;
		*ppath2 = path;
		namelen = strchrnul(path, '/') - path;
		//printk("!!!%.*s", namelen, path);
		if (-1 == dir_find_entry(ip, &de, path, namelen))
			goto err;
		ip = iget(ip->i_dev, de.d_ino);
		if (!ip) {
			printk("WARNING: de dev=%d, ino=%d not exist", ip->i_dev, de.d_ino);
err:
			*ppath = path;
			return NULL;
		}
		if (!ks_target_nofollow || path[namelen])
			ip = ifollow(ip, *pip);
		if (!ip)
			goto err;
	}
	*ppath = path;
	return ip;
}

static struct inode *new_inode(struct inode *pip, mode_t mode, int nod)
{
	struct inode *ip;
	ip = create_inode(pip);
	ip->i_mode = mode;
	if (S_ISDIR(mode)) {
		dir_add_entry(ip, ip, ".", 1);
		dir_add_entry(ip, pip, "..", 2);
	} else if (S_ISNOD(mode)) {
		ip->i_nodnr = nod;
	}
	return ip;
}

int unlinki(const char *path, int rmdir)
{
	int i;
	const char *p, *name;
	struct dir_entry de;
	struct inode *pip, *ip;
	ip = _namei(&path, &pip, &name);
	if (!ip) {
		if (pip) iput(pip);
		return -1;
	}
	if (!pip) {
		iput(ip);
		errno = EINVAL;
		return -1;
	}
	i = dir_find_entry(pip, &de, name, strchrnul(name, '/') - name);
	if (i == -1) {
		printk("WARNING: namei BUG, entry [%s] not found!", name);
		iput(ip);
		iput(pip);
		errno = ENOENT;
		return -1;
	}
	i = dir_del_entry(pip, i, !!rmdir);
	iput(ip);
	iput(pip);
	return i;
}

static struct inode *_creati(
		const char *path, int excl, mode_t mode, int nod,
		struct inode *lip)
{
	size_t namelen;
	const char *p, *unused;
	struct inode *pip, *ip;
	ip = _namei(&path, &pip, &unused);
	if (ip != NULL) {
		if (pip) iput(pip);
		if (excl) {
			iput(ip);
			errno = EEXIST;
			return NULL;
		}
		return ip;
	}
	if (errno != ENOENT)
		return NULL;
	if (!pip) {
		errno = EINVAL;
		return NULL;
	}
	p = strchrnul(path, '/');
	namelen = p - path;
	while (*p == '/')
		p++;
	if (*p) { // no inner directory, /path/_to_/file
		errno = ENOENT;
		goto out;
	}
	if (!S_ISDIR(pip->i_mode)) { // inner one not directory
		errno = ENOTDIR;
		goto out;
	}
	if (lip && pip->i_dev != lip->i_dev) {
		errno = EXDEV;
		goto out;
	}
	if (iaccess(pip, W_OK | X_OK, 0) == -1)
		goto out;
	errno = 0;
	ip = lip ? idup(lip) : new_inode(pip, mode, nod);
	dir_add_entry(pip, ip, path, namelen);
out:
	iput(pip);
	return ip;
}

struct inode *creati(const char *path, int excl, mode_t mode, int nod)
{
	return _creati(path, excl, mode, nod, NULL);
}

int linki(const char *path, struct inode *ip)
{
	if (!ip) panic("linki(NULL)");
	if (S_ISDIR(ip->i_mode)) {
		errno = EISDIR;
		return -1;
	}
	ip = _creati(path, 1, 0, 0, ip);
	if (ip) iput(ip);
	return ip ? 0 : -1;
}

struct inode *namei(const char *path)
{
	const char *unused;
	struct inode *pip, *ip;
	ip = _namei(&path, &pip, &unused);
	if (pip)
		iput(pip);
	return ip;
}

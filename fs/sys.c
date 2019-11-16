#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/fs.h>
#include <errno.h>


static int do_chdir(struct inode *ip)
{
	if (!S_ISDIR(ip->i_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (!S_CHECK(ip->i_mode, S_IXOTH)) {
		errno = EACCES;
		return -1;
	}
	if (current->cwd) iput(current->cwd);
	current->cwd = ip;
	return 0;
}

int sys_chdir(const char *path)
{
	struct inode *ip = namei(path);
	if (!ip) return -1;
	return do_chdir(ip);
}

int sys_mkdir(const char *path, mode_t mode)
{
	struct inode *ip = creati(path, 1, (mode & 0777) | S_IFDIR, 0);
	if (ip) iput(ip);
	return ip ? 0 : -1;
}

int sys_link(const char *path1, const char *path2)
{
	follow_policy_enter(0, 1);
	struct inode *ip = namei(path1);
	follow_policy_leave();
	if (!ip)
		return -1;
	int ret = linki(path2, ip);
	iput(ip);
	return ret;
}


static int alloc_fd(unsigned begin)
{
	for (unsigned i = begin; i < NR_OPEN; i++)
		if (!current->filp[i])
			return i;
	return -1;
}

int sys_open(const char *path, int flags, mode_t mode)
{
	follow_policy_enter(flags & O_NOFOLLOW, flags & O_SYMLINK);
	struct file *f = fs_open(path, flags, mode);
	follow_policy_leave();
	if (!f)
		return -1;
	int fd = alloc_fd(0);
	if (fd == -1) {
		fs_close(f);
		return fd;
	}
	current->filp[fd] = f;
	return fd;
}

int sys_close(int fd)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	fs_close(f);
	current->filp[fd] = NULL;
	return 0;
}

int sys_dup2(int fd, int fd2)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	if ((unsigned)fd2 >= NR_OPEN)
		return -1;
	if (fd == fd2)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	struct file **f2 = &current->filp[fd2];
	if (*f2)
		fs_close(*f2);
	*f2 = fs_dup(f);
	return fd2;
}

int sys_fcntl(int fd, int cmd, int arg)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;

	switch (cmd) {
	case F_DUPFD:
		fd = alloc_fd(arg);
		if (fd == -1)
			return -1;
		current->filp[fd] = fs_dup(f);
		return fd;

	case F_GETFL:
		return f->f_flags;
#if 0
	case F_SETFL:
		f->f_flags = (f->f_flags & ~_O_FLAGS_EDIABLE) | (arg & _O_FLAGS_EDIABLE);
		return 0;
#endif
	case F_SETFD:
		f->f_fdargs = arg;
		return 0;
	case F_GETFD:
		return f->f_fdargs;
	}
	return -1;
}


ssize_t sys_write(int fd, const void *buf, size_t size)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	size = fs_write(f, buf, size);
	if (!size && errno < 0)
		return -1;
	return size;
}

ssize_t sys_read(int fd, void *buf, size_t size)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	errno = 0;
	size = fs_read(f, buf, size);
	if (!size && errno > 0)
		return -1;
	return size;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	return fs_seek(f, offset, whence);
}

int sys_dirread(int fd, struct dirent *de)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	return fs_dirread(f, de);
}

int sys_fstat(int fd, struct stat *st)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	return istat(f->f_ip, st);
}

int sys_pipe(int fd[2])
{
	struct file *fs[2];
	int fd0 = alloc_fd(0);
	if (fd0 == -1)
		return -1;
	int fd1 = alloc_fd(fd0 + 1);
	if (fd1 == -1)
		return -1;
	if (fs_pipe(fs) == -1)
		return -1;
	current->filp[fd0] = fs[0];
	current->filp[fd1] = fs[1];
	fd[0] = fd0;
	fd[1] = fd1;
	return 1;
}


static struct inode *at_old_cwd;

static int at_enter(int fd)
{
	if (fd == AT_FDCWD) {
		at_old_cwd = NULL;
		return 0;
	}
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	at_old_cwd = idup(current->cwd);
	if (do_chdir(idup(f->f_ip)) == -1) {
		iput(at_old_cwd);
		return -1;
	}
	return 0;
}

static void at_leave(void)
{
	if (!at_old_cwd)
		return;
	iput(current->cwd);
	current->cwd = at_old_cwd;
}

static struct inode *at_namei(int fd, const char *path, int flag)
{
	struct inode *ip;
	if (!(flag & AT_EMPTY_PATH) || *path) {
		if (at_enter(fd) == -1)
			return NULL;
		ip = namei(path);
		at_leave();
		return ip;
	}
	if (fd == AT_FDCWD)
		return idup(current->cwd);
	else
		return idup(current->filp[fd]->f_ip);
}

int sys_openat(int fd, const char *path, int flags, mode_t mode)
{
	if (at_enter(fd) == -1)
		return -1;
	fd = sys_open(path, flags, mode);
	at_leave();
	return fd;
}

int sys_fstatat(int fd, const char *path, struct stat *st, int flag)
{
	follow_policy_enter(0, flag & AT_SYMLINK_NOFOLLOW);
	struct inode *ip = at_namei(fd, path, flag);
	follow_policy_leave();
	if (!ip) return -1;
	int ret = istat(ip, st);
	iput(ip);
	return ret;
}

int sys_mkdirat(int fd, const char *path, mode_t mode)
{
	if (at_enter(fd) == -1)
		return -1;
	int ret = sys_mkdir(path, mode);
	at_leave();
	return ret;
}

int sys_unlinkat(int fd, const char *path, int flag)
{
	if (at_enter(fd) == -1)
		return -1;
	int ret = unlinki(path, !!(flag & AT_REMOVEDIR));
	at_leave();
	return ret;
}

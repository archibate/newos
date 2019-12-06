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
	if (iaccess(ip, X_OK, 0) == -1)
		return -1;
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
	errno = EMFILE;
	return -1;
}

static inline int badf(void)
{
	errno = EBADF;
	return -1;
}

int open_fd_object(void *ptr, int type, int flags)
{
	int fd = alloc_fd(0);
	if (fd == -1)
		return -1;
	struct file *f = fs_open_object(ptr, type, flags);
	if (!f)
		return -1;
	current->filp[fd] = f;
	return fd;
}

void *get_fd_object(int fd, int type)
{
	if ((unsigned)fd >= NR_OPEN)
		return NULL;
	struct file *f = current->filp[fd];
	if (!f)
		return NULL;
	if (f->f_type != type)
		return NULL;
	return f->f_ptr;
}

void remove_fd_object(int fd)
{
	current->filp[fd] = 0;
}

int sys_open(const char *path, int flags, mode_t mode)
{
	int fd = alloc_fd(0);
	if (fd == -1)
		return -1;
	follow_policy_enter(flags & O_NOFOLLOW, flags & O_SYMLINK);
	struct file *f = fs_open(path, flags, mode);
	follow_policy_leave();
	if (!f)
		return -1;
	current->filp[fd] = f;
	return fd;
}

int sys_close(int fd)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	fs_close(f);
	current->filp[fd] = NULL;
	return 0;
}

int sys_fcntl(int fd, int cmd, int arg)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();

	switch (cmd) {
	case F_DUPFD_CLOEXEC:
	case F_DUPFD:
		fd = alloc_fd(arg);
		if (fd == -1)
			return -1;
		current->filp[fd] = fs_dup(f);
		if (cmd == F_DUPFD_CLOEXEC)
			current->filp[fd]->f_fdargs |= FD_CLOEXEC;
		else
			current->filp[fd]->f_fdargs &= ~FD_CLOEXEC;
		return fd;

	case F_GETFL:
		return f->f_flags;
	case F_SETFD:
		f->f_fdargs = arg;
		return 0;
	case F_GETFD:
		return f->f_fdargs;
	}
	errno = EINVAL;
	return -1;
}


ssize_t sys_write(int fd, const void *buf, size_t size)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	errno = 0;
	size = fs_write(f, buf, size);
	if (!size && errno < 0)
		return -1;
	return size;
}

ssize_t sys_read(int fd, void *buf, size_t size)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	errno = 0;
	size = fs_read(f, buf, size);
	if (!size && errno > 0)
		return -1;
	return size;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	return fs_seek(f, offset, whence);
}

int sys_dirread(int fd, struct dirent *de)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	return fs_dirread(f, de);
}

int sys_ioctl(int fd, int req, long arg)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	return fs_ioctl(f, req, arg);
}

int sys_ionotify(int fd, int flags)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	return fs_ionotify(f, flags);
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
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
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

int sys_faccessat(int fd, const char *path, int amode, int flag)
{
	follow_policy_enter(0, flag & AT_SYMLINK_NOFOLLOW);
	struct inode *ip = at_namei(fd, path, flag);
	follow_policy_leave();
	if (!ip) return -1;
	int ret = iaccess(ip, amode, flag & AT_EACCESS);
	iput(ip);
	return ret;
}

int sys_mkdirat(int fd, const char *path, mode_t mode)
{
	if (at_enter(fd) == -1)
		return -1;
	struct inode *ip = creati(path, 1, (mode & 0777) | S_IFDIR, 0);
	at_leave();
	if (ip) iput(ip);
	return ip ? 0 : -1;
}

int sys_unlinkat(int fd, const char *path, int flag)
{
	if (at_enter(fd) == -1)
		return -1;
	follow_policy_enter(0, 1);
	int ret = unlinki(path, !!(flag & AT_REMOVEDIR));
	follow_policy_leave();
	at_leave();
	return ret;
}

int sys_linkat(int fd1, const char *path1, int fd2, const char *path2, int flag)
{
	follow_policy_enter(0, !(flag & AT_SYMLINK_FOLLOW));
	struct inode *ip = at_namei(fd1, path1, flag);
	follow_policy_leave();
	if (!ip)
		return -1;
	if (at_enter(fd2) == -1) {
		iput(ip);
		return -1;
	}
	int ret = linki(path2, ip);
	at_leave();
	iput(ip);
	return ret;
}

int sys_ftruncate_s(int fd, size_t length)
{
	if ((unsigned)fd >= NR_OPEN)
		return badf();
	struct file *f = current->filp[fd];
	if (!f)
		return badf();
	return fs_truncate_s(f, length);
}

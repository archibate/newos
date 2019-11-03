#include <kern/sched.h>
#include <kern/kernel.h>
#include <kern/fs.h>
#include <errno.h>

int alloc_fd(void)
{
	for (int i = 0; i < NR_OPEN; i++)
		if (!current->filp[i])
			return i;
	return -1;
}

int sys_open(const char *path, int flags, mode_t mode)
{
	struct file *f = fs_open(path, flags, mode);
	if (!f)
		return -1;
	int fd = alloc_fd();
	if (fd == -1) {
		fs_close(f);
		return fd;
	}
	current->filp[fd] = f;
	return fd;
}

int sys_dup(int fd)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	fd = alloc_fd();
	if (fd == -1)
		return -1;
	current->filp[fd] = fs_dup(f);
	return fd;
}

int sys_dup2(int fd, int fd2)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	if ((unsigned)fd2 >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	struct file **f2 = current->filp + fd2;
	if (*f2)
		return -1;
	*f2 = fs_dup(f);
	return 0;
}

int sys_close(int fd)
{
	if ((unsigned)fd >= NR_OPEN)
		return -1;
	struct file *f = current->filp[fd];
	if (!f)
		return -1;
	fs_close(f);
	return 0;
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

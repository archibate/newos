#include <kern/fs.h>
#include <bits/notify.h>
#include <kern/kernel.h>
#include <kern/sched.h>
#include <malloc.h>
#include <string.h>
#include <string.h>
#include <errno.h>

struct file *fs_open(const char *path, int flags, mode_t mode)
{
	struct inode *ip;
	if (flags & O_CREAT) {
		if (!S_ISDIR(mode) && !S_ISREG(mode) && !S_ISLNK(mode)) {
			errno = EINVAL;
			return NULL;
		}
		if (!!S_ISDIR(mode) != !!(flags & O_DIRECTORY)) {
			errno = EINVAL;
			return NULL;
		}
		ip = creati(path, !!(flags & O_EXCL), mode, 0);
		if (!ip)
			return NULL;
	} else {
		ip = namei(path);
		if (!ip)
			return NULL;
	}
	if ((flags & O_SYMLINK) && !S_ISLNK(ip->i_mode)) {
		errno = EINVAL;
		iput(ip);
		return NULL;
	}
	if (S_ISDIR(ip->i_mode) && !(flags & O_DIRECTORY)) {
		errno = EISDIR;
		iput(ip);
		return NULL;
	}
	if (!S_ISDIR(ip->i_mode) && (flags & O_DIRECTORY)) {
		errno = ENOTDIR;
		iput(ip);
		return NULL;
	}
	if (flags & O_TRUNC) {
		ssize_t size = iseek(ip, 0);
		if (size != -1)
			ip->i_size = size;
		errno = 0;
	}

	struct file *f = calloc(sizeof(struct file), 1);
	f->f_ip = ip;
	f->f_type = FT_INODE;
	f->f_offset = flags & O_APPEND ? ip->i_size : 0;
	f->f_flags = flags;
	if (flags & O_CLOEXEC)
		f->f_fdargs |= FD_CLOEXEC;
	if (iopen(ip, f) == -1) {
		free(f);
		iput(ip);
		return NULL;
	}
	return f;
}

struct file *fs_open_object(void *ptr, int type, int flags)
{
	struct file *f = calloc(sizeof(struct file), 1);
	f->f_type = type;
	f->f_ptr = ptr;
	f->f_offset = 0;
	f->f_flags = flags;
	if (flags & O_CLOEXEC)
		f->f_fdargs |= FD_CLOEXEC;
	return f;
}

struct file *fs_dup(struct file *f)
{
	struct file *f2 = calloc(sizeof(struct file), 1);
	memcpy(f2, f, sizeof(struct file));
	f2->f_ip = idup(f->f_ip);
	return f2;
}

void fs_close(struct file *f)
{
	if (f->f_type == FT_INODE) {
		iput(f->f_ip);
	}
	free(f);
}

size_t fs_read(struct file *f, void *buf, size_t size)
{
	if ((f->f_type != FT_INODE && f->f_type != FT_IMUX) ||
	    (f->f_flags & (O_RDONLY | O_DIRECTORY)) != O_RDONLY) {
		errno = EBADF;
		return 0;
	}
	current->ks_nowait = f->f_flags & O_NONBLOCK;
	size = iread(f->f_ip, f->f_offset, buf, size);
	current->ks_nowait = 0;
	f->f_offset += size;
	return size;
}

size_t fs_write(struct file *f, const void *buf, size_t size)
{
	if ((f->f_type != FT_INODE && f->f_type != FT_IMUX) ||
	    (f->f_flags & (O_WRONLY | O_DIRECTORY)) != O_WRONLY) {
		errno = EBADF;
		return 0;
	}
	struct inode *ip = f->f_type == FT_IMUX ? f->f_wip : f->f_ip;
	current->ks_nowait = f->f_flags & O_NONBLOCK;
	size = iwrite(ip, f->f_offset, buf, size);
	current->ks_nowait = 0;
	f->f_offset += size;
	return size;
}

off_t fs_seek(struct file *f, off_t offset, int whence)
{
	switch (whence) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += f->f_offset;
		break;
	case SEEK_END:
		if (f->f_flags & O_DIRECTORY)
			offset += f->f_ip->i_size / NEFS_DIR_ENTRY_SIZE;
		else
			offset += f->f_ip->i_size;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	offset = iseek(f->f_ip, offset);
	if (offset == -1)
		return -1;
	if (offset < 0)
		offset = 0;
	else if ((size_t)offset > f->f_ip->i_size)
		offset = (off_t)f->f_ip->i_size;
	f->f_offset = offset;
	return offset;
}

int fs_dirread(struct file *f, struct dirent *de)
{
	if (f->f_type != FT_INODE ||
	    (f->f_flags & (O_RDONLY | O_DIRECTORY))
	    != (O_RDONLY | O_DIRECTORY)) {
		errno = EBADF;
		return 0;
	}
	// since dirent = nefs_dir_entry!
	int ret = dir_read_entry(f->f_ip, de, f->f_offset);
	if (ret != -1)
		f->f_offset++;
	return ret;
}

int fs_ioctl(struct file *f, int req, long arg)
{
	if (f->f_type != FT_INODE) {
		errno = ENODEV;
		return -1;
	}
	return iioctl(f->f_ip, req, arg);
}

int fs_ionotify(struct file *f, int flags, long arg)
{
	if (f->f_type == FT_IMUX) {
		int ret = -1 == iionotify(f->f_ip, flags & ~ION_WRITE, arg);
		ret = -1 == iionotify(f->f_wip, flags & ~ION_READ, arg) || ret;
		return ret ? -1 : 0;
	}
	if (f->f_type != FT_INODE) {
		errno = ENODEV;
		return -1;
	}
	int ret = iionotify(f->f_ip, flags, arg);
	return ret;
}

int fs_pipe(struct file *fs[2])
{
	struct inode *ip = make_pipe();
	struct file *fw = calloc(sizeof(struct file), 1);
	struct file *fr = calloc(sizeof(struct file), 1);
	fw->f_ip = ip;
	fw->f_flags = O_WRONLY;
	fr->f_ip = idup(ip);
	fr->f_flags = O_RDONLY;
	fs[0] = fr;
	fs[1] = fw;
	return 0;
}

int fs_truncate_s(struct file *f, size_t length)
{
	if (f->f_type != FT_INODE ||
	    (f->f_flags & (O_RDWR | O_DIRECTORY)) != O_RDWR) {
		errno = EBADF;
		return -1;
	}
	ssize_t size = iseek(f->f_ip, length);
	if (size == -1)
		return -1;
	f->f_ip->i_size = size;
	return 0;
}

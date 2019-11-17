#include <kern/fs.h>
#include <kern/kernel.h>
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
	if (flags & O_TRUNC)
		ip->i_size = 0;

	struct file *f = calloc(sizeof(struct file), 1);
	f->f_ip = ip;
	f->f_offset = flags & O_APPEND ? ip->i_size : 0;
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
	iput(f->f_ip);
	free(f);
}

size_t fs_read(struct file *f, void *buf, size_t size)
{
	if ((f->f_flags & (O_RDONLY | O_DIRECTORY)) != O_RDONLY) {
		errno = EPERM;
		return 0;
	}
	size = iread(f->f_ip, f->f_offset, buf, size);
	f->f_offset += size;
	return size;
}

size_t fs_write(struct file *f, const void *buf, size_t size)
{
	if ((f->f_flags & (O_WRONLY | O_DIRECTORY)) != O_WRONLY) {
		errno = EPERM;
		return 0;
	}
	size = iwrite(f->f_ip, f->f_offset, buf, size);
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
	if (offset < 0)
		offset = 0;
	else if (offset > (off_t)f->f_ip->i_size)
		offset = (off_t)f->f_ip->i_size;
	f->f_offset = offset;
	return offset;
}

int fs_dirread(struct file *f, struct dirent *de)
{
	if ((f->f_flags & (O_RDONLY | O_DIRECTORY)) != (O_RDONLY | O_DIRECTORY)) {
		errno = EPERM;
		return 0;
	}
	// since dirent = nefs_dir_entry!
	int ret = dir_read_entry(f->f_ip, de, f->f_offset);
	if (ret != -1)
		f->f_offset++;
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
	if ((f->f_flags & (O_WRONLY | O_DIRECTORY)) != O_WRONLY) {
		errno = EPERM;
		return -1;
	}
	if (length >= f->f_ip->i_size)
		return 1;
	f->f_ip->i_size = length;
	return 0;
}

#include <kern/fs.h>
#include <kern/kernel.h>
#include <malloc.h>
#include <string.h>

struct file *fs_open(const char *path, int flags, mode_t mode)
{
	struct inode *ip;
	if (flags & O_CREAT) {
		if (!S_ISDIR(mode) && S_ISREG(mode))
			return NULL;
		if (!!S_ISDIR(mode) != !!(flags & O_DIRECTORY))
			return NULL;
		ip = creati(path, !!(flags & O_EXCL), mode, 0);
		if (!ip)
			return NULL;
	} else {
		ip = namei(path);
		if (!ip)
			return NULL;
		if (!!S_ISDIR(ip->i_mode) != !!(flags & O_DIRECTORY)) {
			iput(ip);
			return NULL;
		}
	}
	if (flags & O_TRUNC)
		ip->i_size = 0;

	struct file *f = calloc(sizeof(struct file), 1);
	f->f_ip = ip;
	f->f_offset = flags & O_APPEND ? ip->i_size : 0;
	f->f_flags = flags;
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
	if (!(f->f_flags & O_RDONLY))
		return 0;
	size = iread(f->f_ip, f->f_offset, buf, size);
	f->f_offset += size;
	return size;
}

size_t fs_write(struct file *f, const void *buf, size_t size)
{
	if (!(f->f_flags & O_WRONLY))
		return 0;
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
		offset = f->f_offset + offset;
		break;
	case SEEK_END:
		offset = f->f_ip->i_size + offset;
		break;
	default:
		return -1;
	}
	if (offset < 0)
		offset = 0;
	else if (offset > (off_t)f->f_ip->i_size)
		offset = (off_t)f->f_ip->i_size;
	f->f_offset = offset;
	return offset;
}

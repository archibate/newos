#include <kern/mm.h>
#include <kern/kernel.h>
#include <kern/fs.h>
#include <kern/sched.h>
#include <errno.h>

static void *do_mmap(void *addr, size_t length, int prot, int flags,
                  struct inode *ip, off_t offset)
{
	struct vm_area_struct *vm;
	vm = mm_new_area(current->mm, (viraddr_t)addr, length,
			prot, flags, ip, offset);
	return vm ? addr : MAP_FAILED;
}

void *sys_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
	struct inode *ip;
	length = (length + PGSIZE - 1) & PGMASK;
	if (!length || PGOFFS(addr)) {
		errno = EINVAL;
		return MAP_FAILED;
	}
	if (flags & MAP_ANONYMOUS) {
		ip = NULL;
	} else {
		if ((unsigned)fd >= NR_OPEN) {
			errno = EBADF;
			return MAP_FAILED;
		}
		struct file *f = current->filp[fd];
		if (!f) {
			errno = EBADF;
			return MAP_FAILED;
		}
		if (f->f_flags & O_DIRECTORY) {
			errno = EPERM;
			return MAP_FAILED;
		}
		if (((prot & PROT_READ) && !(f->f_flags & O_RDONLY))
		 || ((prot & PROT_WRITE) && !(f->f_flags & O_WRONLY))) {
			errno = EPERM;
			return MAP_FAILED;
		}
		ip = f->f_ip;
	}
	return do_mmap(addr, length, prot, flags, ip, offset);
}

int sys_munmap(void *addr, size_t length)
{
	length = (length + PGSIZE - 1) & PGMASK;
	if (!length || PGOFFS(addr)) {
		errno = EINVAL;
		return -1;
	}
	mm_find_replace_area(current->mm, (viraddr_t)addr,
			(viraddr_t)addr + length - 1, 0);
	return 0;
}

int sys_msync(void *addr, size_t length, int flags)
{
	length = (length + PGSIZE - 1) & PGMASK;
	if (!length || PGOFFS(addr) || flags != MS_SYNC) {
		errno = EINVAL;
		return -1;
	}
	if (!mm_find_sync_area(current->mm, (viraddr_t)addr,
			(viraddr_t)addr + length - 1)) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

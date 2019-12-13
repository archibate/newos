#include <kern/fs.h>
#include <kern/kernel.h>
#include <bits/notify.h>
#include <kern/sched.h>
#include <malloc.h>
#include <errno.h>

struct inode *make_pipe(void)
{
	struct inode *ip = alloc_m_inode();
	void *p = malloc(PIPE_SIZE);
	ring_init(&ip->i_p_ring, p, PIPE_SIZE);
	ip->i_fstype = IFS_PIPE;
	return ip;
}

void close_pipe(struct inode *ip)
{
	//printk("closepipe %p", ip);
	wake_up(&ip->i_p_read_wait);
	wake_up(&ip->i_p_write_wait);
}

void free_pipe(struct inode *ip)
{
	free(ip->i_p_ring.buf);
}

size_t pipe_read(struct inode *ip, void *buf, size_t size)
{
	size_t s, n, m = 0;
	//printk("pipe_read %d", size);
	while (size > 0) {
		while (!(n = ring_left(&ip->i_p_ring))) {
			wake_up(&ip->i_p_write_wait);
			if (ip->i_count < 2) {
				//printk("reader: writer closed");
				goto out;
			}
			if (sleep_on(&ip->i_p_read_wait))
				goto out;
		}
		s = ip->i_p_ring.size - ip->i_p_ring.head;
		if (s > size)
			s = size;
		if (s > n)
			s = n;
		size -= s;
		m += s;
		n = ip->i_p_ring.head;
		ip->i_p_ring.head += s;
		ip->i_p_ring.head %= ip->i_p_ring.size;
		while (s-- > 0)
			*(char *)buf++ = ip->i_p_ring.buf[n++];
	}
out:
	wake_up(&ip->i_p_write_wait);
	if (m == 0) {
		//printk("reader: EOF!!!");
	}
	return m;
}

size_t pipe_write(struct inode *ip, const void *buf, size_t size)
{
	size_t s, n, m = 0;
	while (size > 0) {
		while (!(n = ring_capacity(&ip->i_p_ring) - ring_left(&ip->i_p_ring))) {
			wake_up(&ip->i_p_read_wait);
			if (ip->i_count < 2) {
				//printk("writer: reader closed");
				current->signal |= _S(SIGPIPE);
				errno = EPIPE;
				goto out;
			}
			if (sleep_on(&ip->i_p_write_wait))
				goto out;
		}
		s = ip->i_p_ring.size - ip->i_p_ring.tail;
		if (s > size)
			s = size;
		if (s > n)
			s = n;
		size -= s;
		m += s;
		n = ip->i_p_ring.tail;
		ip->i_p_ring.tail += s;
		ip->i_p_ring.tail %= ip->i_p_ring.size;
		while (s-- > 0)
			ip->i_p_ring.buf[n++] = *(const char *)buf++;
	}
out:
	wake_up(&ip->i_p_read_wait);
	if (ip->i_p_notify) {
		//printk("pip ionotify ok");
		do_kill(ip->i_p_notify, SIGPOLL, ip->i_p_notify_arg);
		ip->i_p_notify = NULL;
	}
	return m;
}

int pipe_ionotify(struct inode *ip, int flags, long arg)
{
	if (flags & ~ION_READ) {
		errno = EINVAL;
		return -1;
	}
	if (!(flags & ION_READ))
		return 0;
	if (ip->i_p_notify) {
		errno = EAGAIN;
		return -1;
	}
	//printk("pip ionotify hook");
	ip->i_p_notify = current;
	ip->i_p_notify_arg = arg;
	return 0;
}

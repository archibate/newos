#ifndef _KERN_SYSCALL_H
#define _KERN_SYSCALL_H

#include <sys/types.h>
// Get struct dirent.
#include <bits/dirent.h>
// Get struct stat.
#include <bits/stat.h>
// Get struct msqid_ds.
#include <bits/msg.h>
// Get intptr_t.
#include <stdint.h>

#ifndef _SYSCALL
#define _SYSCALL
#endif

#ifdef _DEFINE_KERNEL_SYSCALL_SWITCH
	errno = 0;
	switch (regs[EAX]) {
#define _syscallv(i, rt, name, t1) \
	_SYSCALL rt __attribute__((noreturn)) sys_##name(t1 x1); \
	case (i): sys_##name((t1) regs[EBX]); break;
#define _syscall0(i, rt, name) \
	_SYSCALL rt sys_##name(void); \
	case (i): regs[EAX] = (long)sys_##name(); regs[ECX] = errno; break;
#define _syscall1(i, rt, name, t1) \
	_SYSCALL rt sys_##name(t1); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX]); regs[ECX] = errno; break;
#define _syscallz(i, rt, name, t1) \
	_SYSCALL rt sys_##name(t1); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX]); regs[ECX] = 0; break;
#define _syscall2(i, rt, name, t1, t2) \
	_SYSCALL rt sys_##name(t1, t2); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX], (t2) regs[ECX]); \
		  regs[ECX] = errno; break;
#define _syscall3(i, rt, name, t1, t2, t3) \
	_SYSCALL rt sys_##name(t1, t2, t3); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX], (t2) regs[ECX], (t3) regs[EDX]); \
		  regs[ECX] = errno; break;
#define _syscall4(i, rt, name, t1, t2, t3, t4) \
	_SYSCALL rt sys_##name(t1, t2, t3, t4); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX], (t2) regs[ECX], (t3) regs[EDX], \
				  (t4) regs[ESI]); regs[ECX] = errno; break;
#define _syscall5(i, rt, name, t1, t2, t3, t4, t5) \
	_SYSCALL rt sys_##name(t1, t2, t3, t4, t5); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX], (t2) regs[ECX], (t3) regs[EDX], \
				  (t4) regs[ESI], (t5) regs[EDI]); regs[ECX] = errno; break;
#define _syscall6(i, rt, name, t1, t2, t3, t4, t5, t6) \
	_SYSCALL rt sys_##name(t1, t2, t3, t4, t5, t6); \
	case (i): regs[EAX] = (long)sys_##name((t1) regs[EBX], (t2) regs[ECX], (t3) regs[EDX], \
				  (t4) regs[ESI], (t5) regs[EDI], (t6) regs[EBP]); \
		  regs[ECX] = errno; break;

#else

#ifdef _DEFINE_SYSCALL
#define _DEF_SYS(x) x
#include <errno.h>
#else
#define _DEF_SYS(x) ;
#endif

#define _syscallv(i, rt, name, t1) \
	_SYSCALL rt __attribute__((noreturn)) _##name(t1 x1) \
	_DEF_SYS({ \
		asm volatile ("int $0x80" \
				:: "a" (i), "b" (x1)); \
		for (;;) asm volatile ("ud2"); \
	})
#define _syscall0(i, rt, name) \
	_SYSCALL rt name(void) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i)); \
		return ret; \
	})
#define _syscall1(i, rt, name, t1) \
	_SYSCALL rt name(t1 x1) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1)); \
		if (errno) errno = errno; \
		return ret; \
	})
#define _syscallz(i, rt, name, t1) \
	_SYSCALL rt name(t1 x1) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret) : "a" (i) , "b" (x1)); \
		return ret; \
	})
#define _syscall2(i, rt, name, t1, t2) \
	_SYSCALL rt name(t1 x1, t2 x2) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1), "c" (x2)); \
		return ret; \
	})
#define _syscall3(i, rt, name, t1, t2, t3) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3)); \
		return ret; \
	})
#define _syscall4(i, rt, name, t1, t2, t3, t4) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3, t4 x4) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3), "S" (x4)); \
		return ret; \
	})
#define _syscall5(i, rt, name, t1, t2, t3, t4, t5) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3, t4 x4, t5 x5) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3), "S" (x4) \
				, "D" (x5)); \
		return ret; \
	})
#define _syscall6(i, rt, name, t1, t2, t3, t4, t5, t6) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3, t4 x4, t5 x5, t6 x6) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile (	"push %%ebp\n" \
				"mov %8, %%ebp\n" \
				"int $0x80\n" \
				"pop %%ebp\n" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3), "S" (x4) \
				, "D" (x5), "m" (x6)); \
		return ret; \
	})
#endif

_syscallv(0, void, exit, int);
_syscall3(1, ssize_t, read, int, void *, size_t);
_syscall3(2, ssize_t, write, int, const void *, size_t);
_syscall3(3, off_t, lseek, int, off_t, int);
_syscallz(4, int, close, int);
_syscall3(5, int, fcntl, int, int, long);
_syscall2(6, int, kdebug, int, long);
_syscall3(7, int, open, const char *, int, mode_t);
_syscall0(8, int, pause);
_syscall0(9, pid_t, fork);
_syscall3(10, int, execve, const char *, char *const *, char *const *);
_syscall1(11, void *, sbrk, intptr_t);
_syscall3(12, pid_t, waitpid, pid_t, int *, int);
_syscall0(13, long, sgetmask);
_syscall1(14, long, ssetmask, long);
_syscall3(15, long, signal_r, int, void *, void *);
_syscall3(16, int, kill_a, pid_t, int, long);
_syscall2(17, int, raise_a, int, long);
_syscall3(18, int, mkdirat, int, const char *, mode_t);
_syscall3(19, int, unlinkat, int, const char *, int);
_syscall2(20, int, link, const char *, const char *);
_syscall2(21, int, dirread, int, struct dirent *);
_syscall1(22, int, chdir, const char *);
_syscall4(23, int, openat, int, const char *, int, mode_t);
_syscall0(24, pid_t, getpid);
_syscall0(25, pid_t, getppid);
_syscall4(26, int, fstatat, int, const char *, struct stat *, int);
_syscall4(27, int, faccessat, int, const char *, int, int);
_syscall1(28, int, pipe, int *);
_syscall5(29, int, linkat, int, const char *, int, const char *, int);
_syscall6(30, void *, mmap, void *, size_t, int, int, int, off_t);
_syscall2(31, int, munmap, void *, size_t);
_syscall2(32, int, ftruncate_s, int, size_t);
_syscall3(33, int, ioctl, int, int, long);
_syscall1(34, time_t, alarm, time_t);
_syscall2(35, int, msgget, key_t, int);
_syscall3(36, int, msgctl, int, int, struct msqid_ds *);
_syscall5(37, ssize_t, msgrcv, int, void *, size_t, long, int);
_syscall4(38, int, msgsnd, int, const void *, size_t, int);
_syscall3(39, int, ionotify, int, int, long);
_syscall3(40, int, msync, void *, size_t, int);
_syscall3(41, int, Connect, int, pid_t, id_t);
_syscall1(42, int, CreateChannel, id_t);
struct msgio;
_syscall3(43, int, MsgSend, int, struct msgio *, size_t);
_syscall3(44, int, MsgReceive, id_t, struct msgio *, size_t);

#ifdef _DEFINE_KERNEL_SYSCALL_SWITCH
	default:
		printk("WARNING: undefined syscall %d (%#x)", regs[EAX], regs[EAX]);
		regs[EAX] = -1;
		regs[ECX] = -ENOSYS;
	}
#endif

#endif

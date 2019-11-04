#pragma once

#include <sys/types.h>

#ifndef __user
#define __user
#endif

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
#define _syscall2(i, rt, name, t1, t2) \
	_SYSCALL rt name(t1 x1, t2 x2) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1), "c" (x2)); \
		if (errno) errno = errno; \
		return ret; \
	})
#define _syscall3(i, rt, name, t1, t2, t3) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3)); \
		if (errno) errno = errno; \
		return ret; \
	})
#define _syscall4(i, rt, name, t1, t2, t3, t4) \
	_SYSCALL rt name(t1 x1, t2 x2, t3 x3, t4 x4) \
	_DEF_SYS({ \
		rt ret; \
		asm volatile ("int $0x80" \
				: "=a" (ret), "=c" (errno) : "a" (i) \
				, "b" (x1) , "c" (x2), "d" (x3), "S" (x4)); \
		if (errno) errno = errno; \
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
		if (errno) errno = errno; \
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
_syscall1(4, int, close, int);
_syscall1(5, int, dup, int);
_syscall2(6, int, dup2, int, int);
_syscall3(7, int, open, const char *, int, mode_t);
_syscall0(8, int, pause);
_syscall0(9, pid_t, fork);

#ifdef _DEFINE_KERNEL_SYSCALL_SWITCH
	default:
		printk("WARNING: undefined syscall %d (%#x)", regs[EAX], regs[EAX]);
		regs[EAX] = -1;
		regs[ECX] = -ENOSYS;
	}
#endif

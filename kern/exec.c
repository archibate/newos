#include <kern/exec.h>
#include <kern/kernel.h>
#include <string.h>
#include <malloc.h>

static size_t length_arr2d(char *const *src)
{
	size_t n = 0;
	while (*src++)
		n++;
	return n;
}

static char **dup_arr2d(char *const *src)
{
	size_t len = length_arr2d(src);
	char **dst = malloc(sizeof(char *) * (len + 1));
	char **p = dst;
	while (len-- > 0)
		*p++ = strdup(*src++);
	*p = NULL;
	return dst;
}

static void *push_free1_arr2d(void *sp, char **src, size_t *cnt)
{
	char **p = src;
	size_t size;
	while (*p) {
		sp -= strlen(*p) + 1;
		strcpy(sp, *p);
		free(*p);
		*p++ = sp;
	}
	*cnt = p - src;
	sp = (void *)((uintptr_t)sp & -7);
	size = (p - src + 1) * sizeof(char *);
	sp -= size;
	memcpy(sp, src, size);
	return sp;
}

static int __do_execve(struct inode *ip, char **argv, char **envp)
{
	size_t argc, envc;
	struct mm_struct *mm = create_mm();
	if (mm_load_exec(mm, task_regs(current), ip) == -1)
		return -1;
	if (current->mm)
		free_mm(current->mm);
	switch_to_mm(current->mm = mm);
	void **psp = (void **)&task_regs(current)[ESP];
	argv = *psp = push_free1_arr2d(*psp, argv, &argc);
	envp = *psp = push_free1_arr2d(*psp, envp, &envc);
	unsigned long *sp = *psp;
	*--sp = envc;
	*--sp = (unsigned long)envp;
	*--sp = (unsigned long)argv;
	*--sp = argc;
	*--sp = 0x233;
	*psp = sp;
	return 0;
}

int do_execve(struct inode *ip, char *const *argv, char *const *envp)
{
	char **kargv = dup_arr2d(argv);
	char **kenvp = dup_arr2d(envp);
	int ret = __do_execve(ip, kargv, kenvp);
	free(kargv);
	free(kenvp);
	return ret;
}

int sys_execve(const char *path, char *const *argv, char *const *envp)
{
	struct inode *ip = namei(path);
	if (!ip) return -1;
	int ret = do_execve(ip, argv, envp);
	iput(ip);
	if (!ret) move_to_user();
	return ret;
}

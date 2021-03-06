#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H 1

#include <bits/wait.h>
#include <bits/types.h>

#ifndef pid_t
#define pid_t __pid_t
#endif
#ifndef id_t
#define id_t __id_t
#endif

pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

#endif

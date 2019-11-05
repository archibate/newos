#include <signal.h>

extern void __signal_restorer(void);

__sighandler_t signal(int sig, __sighandler_t handler)
{
	return signal_r(sig, handler, __signal_restorer);
}

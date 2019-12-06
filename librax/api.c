#include <sys/ipc.h>
#include <sys/msg.h>

static int g_msq, g_msq_r;

int XOpenServer(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1)
		return -1;
	key_t key_r = ftok("/dev/fb0", 6666);
	if (key == -1)
		return -1;
	g_msq = msgget(key, 0);
	if (g_msq == -1)
		return -1;
	g_msq_r = msgget(key_r, 0);
	if (g_msq_r == -1)
		return -1;
	return 0;
}

#include "idl/rax.c"

#include <sys/ipc.h>
#include <sys/msg.h>

static int g_msq;

int XOpenServer(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1)
		return -1;
	g_msq = msgget(key, 0);
	if (g_msq == -1)
		return -1;
	return 0;
}

#include "idl/rax.c"

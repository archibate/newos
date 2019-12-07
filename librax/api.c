#include <rax/rax.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h> /* idl needs that... */

static int g_msq, g_msq_r, g_msq_m;

int XClientInit(void)
{
	key_t key = ftok("/dev/fb0", 2333);
	if (key == -1)
		return -1;
	key_t key_r = ftok("/dev/fb0", 6666);
	key_t key_m = ftok("/dev/fb0", 8888);
	g_msq = msgget(key, 0);
	if (g_msq == -1)
		return -1;
	g_msq_r = msgget(key_r, 0);
	if (g_msq_r == -1)
		return -1;
	g_msq_m = msgget(key_m, 0);
	if (g_msq_m == -1)
		return -1;
	return 0;
}

int XListen(int hlst, struct Message *msg)
{
	return msgrcv(g_msq_m, msg,
			sizeof(*msg) - sizeof(msg->hlst),
			hlst, 0);
}

#include "idl/rax.c"

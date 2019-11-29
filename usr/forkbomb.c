#include "busybox.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

static void fuck(void)
{
	char hell[23333];
	memset(hell, 233, sizeof(hell));
	malloc(23333);
}

int main(int argc, char **argv)
{
	while (1) {
		fork();
		fuck();
	}
}

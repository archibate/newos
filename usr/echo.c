#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc <= 1)
		putchar('\n');
	else for (int i = 1; i < argc; i++)
		printf("%s%c", argv[i], " \n"[i == argc - 1]);
	return 0;
}

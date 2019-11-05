#include <stdio.h>

int main(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		printf("%s%c", argv[i], " \n"[i == argc - 1]);
	}
}

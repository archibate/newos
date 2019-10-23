#define true
true /* ; chmod +x $0; gcc -lpthread $0 -o /tmp/$$ && /tmp/$$ $*; exit; */
#undef true
#include <stdio.h>
#include <stdlib.h>

#define SEC 512

int main(int argc, char **argv)
{
	if (!argc) {
		fprintf(stderr, "no image file specified\n");
		return EXIT_FAILURE;
	}
	FILE *fp = fopen(argv[1], "r+");
	if (!fp) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	size_t pos = ftell(fp), npos;
	npos = (pos + SEC - 1) & -SEC;

	printf("fixsect %#x -> %#x\n", pos, npos);

	for (; pos < npos; pos++)
		fputc(0, fp);

	fclose(fp);
	return EXIT_SUCCESS;
}

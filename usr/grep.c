#include <stdio.h>
#include <string.h>

int found, opt_v, show_prefix;

int grep(const char *exp, const char *path)
{
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");
	if (!fp) {
		perror(path);
		return 1;
	}

	char buf[2048];
	while (fgets(buf, sizeof(buf), fp)) {
		if (opt_v != !!strstr(buf, exp)) {
			found = 1;
			if (show_prefix)
				printf("%s:", path);
			fputs(buf, stdout);
		}
	}

	if (fp != stdin) fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc >= 2) {
		if (!strcmp(argv[1], "-v")) {
			opt_v = 1;
			argc--;
			argv++;
		}
	}
	if (argc < 2) {
		fprintf(stderr, "grep: missing oprand\n");
		return 0;
	}
	int err = 0;
	if (argc == 2) {
		err = grep(argv[1], "-");
	} else {
		show_prefix = 1;
		for (int i = 2; i < argc; i++)
			err = grep(argv[1], argv[i]) || err;
	}
	return err ? 2 : found;
}

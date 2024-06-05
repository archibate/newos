#include <stdio.h>
#include <getopt.h>
#include <string.h>

static struct _getopt __getopt;

struct _getopt *__getopt_location() {
    return &__getopt;
}

static int optinn = 0;

int getopt(int argc, char *const *argv, const char *optstr)
{
	int c;
	char *s;
	const char *p;
	optarg = NULL;
	if (optind >= argc)
		return -1;
	s = argv[optind];
	if (!optinn) {
		optinn = 1;
		while (!(s[0] == '-' && s[1])) {
			optind++;
			if (optstr[0] == '-') {
				optarg = s;
				return 1;
			}
			if (optind >= argc)
				return -1;
			s = argv[optind];
		}
		if (s[1] == '-' && !s[2])
			return -1;
	}
	c = s[optinn++];
	if (!s[optinn])
		optinn = 0, optind++;
	p = strchr(optstr, c);
	if (!p) {
		optopt = c;
		if (opterr)
			fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
		return '?';
	}
	if (p[1] != ':')
		return c;
	if (optinn) {
		optarg = s + optinn;
		optinn = 0, optind++;
		return c;
	}
	if (optind >= argc) {
		optopt = c;
		if (opterr)
			fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], c);
		return '?';
	}
	optarg = argv[optind];
	optind++;
	return c;
}

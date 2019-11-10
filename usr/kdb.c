#include <stdio.h>
#include <kdebug.h>

int kdb(const char *s)
{
	int err = 0;
	while (*s == '-')
		s++;
	for (; *s; s++) {
		switch (*s) {
		case 'I': case 'i':	err = err || kdebug(KDB_DUMP_INODE, *s == 'I'); break;
		case 's':		err = err || kdebug(KDB_DUMP_SUPER); break;
		case 'B': case 'b':	err = err || kdebug(KDB_DUMP_BUFFER, *s == 'B'); break;
		case 'M': case 'm':	err = err || kdebug(KDB_DUMP_KHEAP, *s == 'M'); break;
		case 'p':		err = err || kdebug(KDB_DUMP_TASKS); break;
		case 'h':		err = err || kdebug(KDB_HALT); break;
		default: fprintf(stderr, "kdb: bad option - '%c'\n", *s); 
		}
	}
	return err;
}

int main(int argc, char **argv)
{
	if (kdebug(KDB_TEST) == -1) {
		fprintf(stderr, "kdb: kdebug not enabled\n");
		return 1;
	}
	int err = 0;
	for (int i = 1; i < argc; i++)
		err = err || kdb(argv[i]);
	return err;
}

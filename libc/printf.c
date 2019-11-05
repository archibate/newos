#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>

int vasprintf(char ** buf, const char * fmt, va_list args) {
	char * b = malloc(1024);
	*buf = b;
	return vsnprintf(b, 1024, fmt, args);
}

int vfprintf(FILE * device, const char *fmt, va_list args) {
	char * buffer;
	vasprintf(&buffer, fmt, args);

	int out = fputs(buffer, device);
	free(buffer);
	return out;
}

int vprintf(const char *fmt, va_list args) {
	return vfprintf(stdout, fmt, args);
}

int fprintf(FILE * device, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char * buffer;
	vasprintf(&buffer, fmt, args);
	va_end(args);

	int out = fputs(buffer, device);
	free(buffer);
	return out;
}

int printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char * buffer;
	vasprintf(&buffer, fmt, args);
	va_end(args);
	int out = fputs(buffer, stdout);
	free(buffer);
	return out;
}


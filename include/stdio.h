#ifndef _STDIO_H
#define _STDIO_H 1

// Get size_t, NULL from stddef.h
#include <stddef.h>
// Get va_list.
#include <stdarg.h>
// Get SEEK_*
#include <bits/unistd.h>

#define BUFSIZ 8192
#define EOF (-1)

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

typedef struct _io_buf FILE;

typedef struct _fpos
{
	long __pos;
} fpos_t;

extern FILE *stdin, *stdout, *stderr;

int printf(const char *fmt, ...);
int fprintf(FILE *fp, const char *fmt, ...);
int dprintf(int fd, const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);
int snprintf(char *str, size_t n, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int vfprintf(FILE *fp, const char *fmt, va_list ap);
int vdprintf(int fd, const char *fmt, va_list ap);
int vsprintf(char *str, const char *fmt, va_list ap);
int vsnprintf(char *str, size_t n, const char *fmt, va_list ap);

FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *fp);
FILE *fdopen(int fd, const char *mode);
int setvbuf(FILE *fp, char *buf, int mode, size_t size);
int ungetc(int c, FILE *fp);
int _fwouldblock(FILE *fp);
int fclose(FILE *fp);
int fseek(FILE *fp, long offset, int whence);
long ftell(FILE *fp);
int fgetpos(FILE *fp, fpos_t *pos);
int fsetpos(FILE *fp, const fpos_t *pos);
size_t fread(void *buf, size_t size, size_t nmemb, FILE *fp);
size_t fwrite(const void *buf, size_t size, size_t nmemb, FILE *fp);
int fileno(FILE *fp);
int fflush(FILE *fp);
int fputs(const char *s, FILE *fp);
int puts(const char *s);
int fputc(int c, FILE *fp);
int fgetc(FILE *fp);
#define getc fgetc
#define putc fputc
int getchar(void);
int putchar(int c);
char *fgets(char *s, int size, FILE *stream);
void rewind(FILE *fp);
void setbuf(FILE *fp, char *buf);
void setbuffer(FILE *fp, char *buf);
int feof(FILE *fp);
void clearerr(FILE *fp);
int ferror(FILE *fp);

#endif

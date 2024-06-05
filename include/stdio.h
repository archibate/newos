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
#define _IOEOF 4
#define _IOERR 8
#define _IOMYBUF 16

typedef struct _io_buf FILE;
typedef struct _fpos {
	long __pos;
} fpos_t;

extern FILE *__iob[3];
#define stdin  (__iob[0])
#define stdout (__iob[1])
#define stderr (__iob[2])
#define fgetc getc
#define fputc putc

int printf(const char *fmt, ...);
int fprintf(FILE *f, const char *fmt, ...);
int dprintf(int fd, const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);
int snprintf(char *str, size_t n, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int vfprintf(FILE *f, const char *fmt, va_list ap);
int vdprintf(int fd, const char *fmt, va_list ap);
int vsprintf(char *str, const char *fmt, va_list ap);
int vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
void perror(const char *msg);

FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *f);
FILE *fdopen(int fd, const char *mode);
int ungetc(int c, FILE *f);
int fwouldblock(FILE *f);
int fclose(FILE *f);
int fseek(FILE *f, long offset, int whence);
long ftell(FILE *f);
int fgetpos(FILE *f, fpos_t *pos);
int fsetpos(FILE *f, const fpos_t *pos);
size_t fread(void *buf, size_t size, size_t nmemb, FILE *f);
size_t fwrite(const void *buf, size_t size, size_t nmemb, FILE *f);
int fileno(FILE *f);
int fflush(FILE *f);
int fputs(const char *s, FILE *f);
int puts(const char *s);
int fputc(int c, FILE *f);
int fgetc(FILE *f);
int getchar(void);
int putchar(int c);
char *fgets(char *s, int size, FILE *stream);
void rewind(FILE *f);
int setvbuf(FILE *f, char *buf, int mode, size_t size);
void setbuffer(FILE *f, char *buf, size_t size);
void setbuf(FILE *f, char *buf);
int feof(FILE *f);
void clearerr(FILE *f);
int ferror(FILE *f);

#endif

// Stripped-down primitive printf-style formatting routines,
// used in common by printf, sprintf, fprintf, etc.
// This code is also used by both the kernel and user programs.

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>

/*
 * Space or zero padding and a field width are supported for the numeric
 * formats only.
 *
 * The special format %e takes an integer error code
 * and prints a string describing the error.
 * The integer may be positive or negative,
 * so that -ENOMEM and ENOMEM are equivalent.
 */

static const char *const error_string[] =
{ // {{{
	[0] = "Success",
	[EPERM] = "Operation not permitted",
	[ENOENT] = "No such file or directory",
	[ESRCH] = "No such process",
	[EINTR] = "Interrupted system call",
	[EIO] = "Input/Output error",
	[ENXIO] = "No such device or address",
	[E2BIG] = "Argument list too long",
	[ENOEXEC] = "Exec format error",
	[EBADF] = "Bad file descriptor",
	[ECHILD] = "No child processes",
	[EAGAIN] = "Resource temporarily unavailable",
	[ENOMEM] = "Cannot allocate memory",
	[EACCES] = "Permission denied",
	[EFAULT] = "Bad address",
	[ENOTBLK] = "Block device required",
	[EBUSY] = "Device or resource busy",
	[EEXIST] = "File exists",
	[EXDEV] = "Invalid cross-device link",
	[ENODEV] = "No such device",
	[ENOTDIR] = "Not a directory",
	[EISDIR] = "Is a directory",
	[EINVAL] = "Invalid argument",
	[ENFILE] = "Too many open files in system",
	[EMFILE] = "Too many open files",
	[ENOTTY] = "Inappropriate ioctl for device",
	[ETXTBSY] = "Text file busy",
	[EFBIG] = "File too large",
	[ENOSPC] = "No space left on device",
	[ESPIPE] = "Illegal seek",
	[EROFS] = "Read only file system",
	[EMLINK] = "Too many links",
	[EPIPE] = "Broken pipe",
	[EDOM] = "Math arg out of domain of func",
	[ERANGE] = "Math result not representable",
	[ENOMSG] = "No message of desired type",
	[EIDRM] = "Identifier removed",
	[ECHRNG] = "Channel number out of range",
	[EL2NSYNC] = "Level 2 not synchronized",
	[EL3HLT] = "Level 3 halted",
	[EL3RST] = "Level 3 reset",
	[ELNRNG] = "Link number out of range",
	[EUNATCH] = "Protocol driver not attached",
	[ENOCSI] = "No CSI structure available",
	[EL2HLT] = "Level 2 halted",
	[EDEADLK] = "Deadlock condition",
	[ENOLCK] = "No record locks available",
	[EBADE] = "Invalid exchange",
	[EBADR] = "Invalid request descriptor",
	[EXFULL] = "Exchange full",
	[ENOANO] = "No anode",
	[EBADRQC] = "Invalid request code",
	[EBADSLT] = "Invalid slot",
	[EBFONT] = "Bad font file format",
	[ENOSTR] = "Device not a stream",
	[ENODATA] = "No data available",
	[ETIME] = "Timer expired",
	[ENOSR] = "Out of streams resources",
	[ENONET] = "Machine is not on the network",
	[ENOPKG] = "Package not installed",
	[EREMOTE] = "The object is remote",
	[ENOLINK] = "The link has been severed",
	[EADV] = "Advertise error",
	[ESRMNT] = "Srmount error",
	[ECOMM] = "Communication error on send",
	[EPROTO] = "Protocol error",
	[EMULTIHOP] = "Multihop attempted",
	[EDOTDOT] = "Cross mount point",
	[EBADMSG] = "Bad message",
	[ENOTUNIQ] = "Name not unique",
	[EBADFD] = "File descriptor in bad state",
	[EREMCHG] = "Remote address changed",
	[ELIBACC] = "Can not access a needed shared library",
	[ELIBBAD] = "Accessing a corrupted shared library",
	[ELIBSCN] = ".lib section in a.out corrupted",
	[ELIBMAX] = "Attempting to link in too many libraries",
	[ELIBEXEC] = "Attempting to exec a shared library",
	[ENOSYS] = "Function not implemented",
	[ENOTEMPTY] = "Directory not empty",
	[ENAMETOOLONG] = "File or path name too long",
	[ELOOP] = "Too many symbolic links",
	[EOPNOTSUPP] = "Operation not supported on transport endpoint",
	[EPFNOSUPPORT] = "Protocol family not supported",
	[ECONNRESET] = "Connection reset by peer",
	[ENOBUFS] = "No buffer space available",
	[EAFNOSUPPORT] = "Address family not supported by protocol family",
	[EPROTOTYPE] = "Protocol wrong type for socket",
	[ENOTSOCK] = "Socket operation on non-socket",
	[ENOPROTOOPT] = "Protocol not available",
	[ESHUTDOWN] = "Can't send after socket shutdown",
	[ECONNREFUSED] = "Connection refused",
	[EADDRINUSE] = "Address already in use",
	[ECONNABORTED] = "Connection aborted",
	[ENETUNREACH] = "Network is unreachable",
	[ENETDOWN] = "Network interface is not configured",
	[ETIMEDOUT] = "Connection timed out",
	[EHOSTDOWN] = "Host is down",
	[EHOSTUNREACH] = "No route to host",
	[EINPROGRESS] = "Connection already in progress",
	[EALREADY] = "Socket already connected",
	[EDESTADDRREQ] = "Destination address required",
	[EMSGSIZE] = "Message too long",
	[EPROTONOSUPPORT] = "Unknown protocol",
	[ESOCKTNOSUPPORT] = "Socket type not supported",
	[EADDRNOTAVAIL] = "Address not available",
	[EISCONN] = "Socket is already connected",
	[ENOTCONN] = "Socket is not connected",
	[EOVERFLOW] = "Value too large for defined data type",
	[ECANCELED] = "Operation canceled",
	[ENOTRECOVERABLE] = "State not recoverable",
	[EOWNERDEAD] = "Previous owner died",
	[ESTRPIPE] = "Streams pipe error",
}; // }}}

static const char *const table[2] =
{
	"0123456789abcdef",
	"0123456789ABCDEF",
};

/*
 * Print a number (base <= 16) in reverse order,
 * using specified putch function and associated pointer putdat.
 */
static void
printnum(void (*putch)(int, void*), void *putdat, const char *table,
		unsigned long long num, unsigned base, int width, int padc)
{
	// first recursively print all preceding (more significant) digits
	if (num >= base) {
		printnum(putch, putdat, table, (unsigned long)num / base, base, width - 1, padc);
	} else {
		// print any needed pad characters before first digit
		while (--width > 0)
			putch(padc, putdat);
	}

	// then print this (the least significant) digit
	putch(table[(unsigned long)num % base], putdat);
}

// Get an unsigned int of various possible sizes from a varargs list,
// depending on the lflag parameter.
static unsigned long long
getuint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, unsigned long long);
	else if (lflag)
		return va_arg(*ap, unsigned long);
	else
		return va_arg(*ap, unsigned int);
}

// Same as getuint but signed - can't use getuint
// because of sign extension
static long long
getint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, long long);
	else if (lflag)
		return va_arg(*ap, long);
	else
		return va_arg(*ap, int);
}


// Main function to format and print a string.
void printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);

void
vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list ap)
{
	register const char *p;
	register int ch, err;
	unsigned long long num;
	int base, lflag, width, precision, altflag;
	char padc;

	while (1) {
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0')
				return;
			putch(ch, putdat);
		}

		// Process a %-escape sequence
		padc = ' ';
		width = -1;
		precision = -1;
		lflag = 0;
		altflag = 0;
reswitch:
		switch (ch = *(unsigned char *) fmt++) {

			// flag to pad on the right
			case '-':
				padc = '-';
				goto reswitch;

				// flag to pad with 0's instead of spaces
			case '0':
				padc = '0';
				goto reswitch;

				// width field
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				for (precision = 0; ; ++fmt) {
					precision = precision * 10 + ch - '0';
					ch = *fmt;
					if (ch < '0' || ch > '9')
						break;
				}
				goto process_precision;

			case '*':
				precision = va_arg(ap, int);
				goto process_precision;

			case '.':
				if (width < 0)
					width = 0;
				goto reswitch;

			case '#':
				altflag = 1;
				goto reswitch;

process_precision:
				if (width < 0)
					width = precision, precision = -1;
				goto reswitch;

				// long flag (doubled for long long)
			case 'l':
				lflag++;
				goto reswitch;

				// character
			case 'c':
				putch(va_arg(ap, int), putdat);
				break;

				// error message
			case 'e':
				err = va_arg(ap, int);
				if (err < 0)
					err = -err;
				if ((unsigned)err >= array_sizeof(error_string)
						|| (p = error_string[err]) == NULL)
					printfmt(putch, putdat, "error %d", err);
				else
					printfmt(putch, putdat, "%s", p);
				break;

				// string
			case 's':
				if ((p = va_arg(ap, char *)) == NULL)
					p = "(null)";
				if (width > 0 && padc != '-')
					for (width -= strnlen(p, precision); width > 0; width--)
						putch(padc, putdat);
				for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0); width--)
					if (altflag && (ch < ' ' || ch > '~'))
						putch('?', putdat);
					else
						putch(ch, putdat);
				for (; width > 0; width--)
					putch(' ', putdat);
				break;

				// (signed) decimal
			case 'd':
				num = getint(&ap, lflag);
				if ((long long) num < 0) {
					putch('-', putdat);
					num = -(long long) num;
				}
				base = 10;
				goto number;

				// unsigned decimal
			case 'u':
				num = getuint(&ap, lflag);
				base = 10;
				goto number;

				// (unsigned) octal
			case 'o':
				if (altflag)
					putch('0', putdat);
				num = getuint(&ap, lflag);
				base = 8;
				goto number;

				// pointer
			case 'p':
				width = sizeof(unsigned long) * 2;
				padc = '0';
				if (altflag)
					putch('0', putdat);
				putch('0', putdat);
				putch('x', putdat);
				num = (unsigned long long)
					(unsigned long) va_arg(ap, void *);
				base = 16;
				goto number;

				// (unsigned) hexadecimal, 'X' for captial character
			case 'X':
			case 'x':
				if (altflag) {
					putch('0', putdat);
					putch('x', putdat);
				}
				num = getuint(&ap, lflag);
				base = 16;
number:
				printnum(putch, putdat, table[ch == 'X'], num, base, width, padc);
				break;

				// escaped '%' character
			case '%':
				putch(ch, putdat);
				break;

				// unrecognized escape sequence - just print it literally
			default:
				putch('%', putdat);
				for (fmt--; fmt[-1] != '%'; fmt--)
					/* do nothing */;
				break;
		}
	}
}

void
printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintfmt(putch, putdat, fmt, ap);
	va_end(ap);
}

struct sprintbuf {
	char *buf;
	char *ebuf;
	int cnt;
};

static void
sprintputch(int ch, struct sprintbuf *b)
{
	b->cnt++;
	if (b->buf < b->ebuf)
		*b->buf++ = ch;
}

int
vsnprintf(char *buf, size_t n, const char *fmt, va_list ap)
{
	struct sprintbuf b = {buf, buf+n-1, 0};

	if (buf == NULL || n < 1)
		return -1;

	// print the string to the buffer
	vprintfmt((void*)sprintputch, &b, fmt, ap);

	// null terminate the buffer
	*b.buf = '\0';

	return b.cnt;
}

int
snprintf(char *buf, size_t n, const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = vsnprintf(buf, n, fmt, ap);
	va_end(ap);

	return rc;
}

int
vsprintf(char *buf, const char *fmt, va_list ap)
{
	return vsnprintf(buf, 0x7fffffff, fmt, ap);
}

int
sprintf(char *buf, const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = vsprintf(buf, fmt, ap);
	va_end(ap);

	return rc;
}

#include <stdio.h>
#include <string.h>
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

char *strerror(int err)
{
	if (err < 0)
		err = -err;
	if ((unsigned)err < array_sizeof(error_string) && error_string[err])
		return (char *)error_string[err];

	static char buf[20];
	snprintf(buf, 20, "error %d", err);
	return buf;
}

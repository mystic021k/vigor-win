/* Forced in before every translation unit (/FI). */
#ifndef VIGOR_FORCED_H
#define VIGOR_FORCED_H

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_DECLARE_NONSTDC_NAMES 1

/* nvi picks stdarg prototypes when this is set. */
#ifndef __STDC__
#define __STDC__ 1
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

/* UCRT rejects a zero buffer size; BSD treats 0 as "pick a size". */
int vigor_setvbuf(FILE *, char *, int, size_t);
#define setvbuf(stream, buffer, mode, size) \
	vigor_setvbuf((stream), (buffer), (mode), (size_t)(size))

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef int pid_t;
#endif

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef int ssize_t;
#endif

#ifndef _UID_T_DEFINED
#define _UID_T_DEFINED
typedef int uid_t;
typedef int gid_t;
#endif

#ifndef _MODE_T_DEFINED
#define _MODE_T_DEFINED
typedef unsigned short mode_t;
#endif

typedef char *caddr_t;

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define S_IXUSR _S_IEXEC
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#define S_IWGRP 0
#define S_IXGRP 0
#define S_IROTH 0
#define S_IWOTH 0
#define S_IXOTH 0
#define S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define S_IRWXG 0
#define S_IRWXO 0
#define S_ISVTX 0
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & _S_IFMT) == _S_IFCHR)
#endif
#ifndef _S_IFIFO
#define _S_IFIFO 0x1000
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(m) (((m) & _S_IFMT) == _S_IFIFO)
#endif

#ifndef O_ACCMODE
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
/* Must be zero: the CRT rejects unknown open bits. */
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

#ifndef F_GETFD
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#endif
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif

#ifndef SIGHUP
#define SIGHUP 1
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif
#ifndef SIGKILL
#define SIGKILL 9
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#ifndef SIGALRM
#define SIGALRM 14
#endif
#ifndef SIGUSR1
#define SIGUSR1 16
#endif
#ifndef SIGUSR2
#define SIGUSR2 17
#endif
#ifndef SIGCHLD
#define SIGCHLD 18
#endif
#ifndef SIGCONT
#define SIGCONT 19
#endif
#ifndef SIGSTOP
#define SIGSTOP 20
#endif
#ifndef SIGTSTP
#define SIGTSTP 23
#endif
#ifndef SIGTTIN
#define SIGTTIN 24
#endif
#ifndef SIGTTOU
#define SIGTTOU 25
#endif
#ifndef SIGWINCH
#define SIGWINCH 28
#endif

char *strsep(char **, const char *);
int mkstemp(char *);
int setenv(const char *, const char *, int);
int unsetenv(const char *);

#endif

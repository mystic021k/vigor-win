/* @(#)port.h.in adapted for the MSVC terminal build. */
#ifndef VIGOR_PORT_H
#define VIGOR_PORT_H

#include <unistd.h>

#undef __P
#define __P(protos) protos

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

#ifndef SEEK_END
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif

#ifndef TCSASOFT
#define TCSASOFT 0
#endif

#ifndef MAXPATHLEN
#ifdef PATH_MAX
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 1024
#endif
#endif

#ifndef MAX
#define MAX(_a,_b) ((_a)<(_b)?(_b):(_a))
#endif
#ifndef MIN
#define MIN(_a,_b) ((_a)<(_b)?(_a):(_b))
#endif

/* Keep the private DB names nvi's configure step rewrites. */
#define DB L__DB
#undef pgno_t
#define pgno_t L__db_pgno_t

#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

#ifndef EFTYPE
#define EFTYPE EINVAL
#endif

#ifndef _POSIX2_RE_DUP_MAX
#define _POSIX2_RE_DUP_MAX 255
#endif

#ifndef WCOREDUMP
#define WCOREDUMP(a) 0
#endif

#if !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN 1234
#endif
#if !defined(BIG_ENDIAN)
#define BIG_ENDIAN 4321
#endif
#if !defined(BYTE_ORDER)
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#endif

/* Hand-written stand-in for the autoconf config.h. Terminal nvi only. */
#ifndef VIGOR_CONFIG_H
#define VIGOR_CONFIG_H

#define STDC_HEADERS 1

#define HAVE_BSEARCH 1
#define HAVE_GETHOSTNAME 1
#define HAVE_MEMCHR 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_SELECT 1
#define HAVE_SNPRINTF 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRPBRK 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE_VSNPRINTF 1
#define HAVE_STRSEP 1
#define HAVE_SETENV 1
#define HAVE_UNSETENV 1
#define HAVE_MKSTEMP 1

#define HAVE_BSD_CURSES 1
#define HAVE_CURSES_ADDNSTR 1
#define HAVE_CURSES_IDLOK 1

#define HAVE_LOCK_FLOCK 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_MMAN_H 1

/* Tcl/Tk and Perl are not on this machine. */
/* #undef HAVE_TCL_INTERP */
/* #undef HAVE_PERL_INTERP */
/* #undef HAVE_MMAP */
/* #undef HAVE_SYS5_PTY */
/* #undef HAVE_ST_BLKSIZE */
/* #undef HAVE_VFORK_H */
/* #undef HAVE_BROKEN_GETTIMEOFDAY */
/* #undef HAVE_BROKEN_VDISABLE */
/* #undef WORDS_BIGENDIAN */
/* #undef SPRINTF_RET_CHARPNT */

#endif

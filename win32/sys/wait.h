#ifndef VIGOR_SYS_WAIT_H
#define VIGOR_SYS_WAIT_H

#include <sys/types.h>

#define WNOHANG 1
#define WUNTRACED 2

#define WIFEXITED(s) (((s) & 0x7f) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 0xff)
#define WIFSIGNALED(s) (((s) & 0x7f) != 0 && ((s) & 0x7f) != 0x7f)
#define WTERMSIG(s) ((s) & 0x7f)
#ifndef WCOREDUMP
#define WCOREDUMP(s) 0
#endif

pid_t waitpid(pid_t, int *, int);

#endif

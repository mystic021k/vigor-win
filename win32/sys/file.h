#ifndef VIGOR_SYS_FILE_H
#define VIGOR_SYS_FILE_H

#include <fcntl.h>

#define LOCK_SH 0x1
#define LOCK_EX 0x2
#define LOCK_NB 0x4
#define LOCK_UN 0x8

int flock(int, int);

#endif

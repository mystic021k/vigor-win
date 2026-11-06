#ifndef VIGOR_SYS_MMAN_H
#define VIGOR_SYS_MMAN_H

#include <sys/types.h>

#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define MAP_SHARED  1
#define MAP_PRIVATE 2
#define MAP_FILE    0

void *mmap(void *, size_t, int, int, int, long);
int munmap(void *, size_t);

#endif

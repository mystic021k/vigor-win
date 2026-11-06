#ifndef VIGOR_SYS_SELECT_H
#define VIGOR_SYS_SELECT_H

#define FD_SETSIZE 64

typedef struct {
	unsigned long bits[(FD_SETSIZE + 31) / 32];
} fd_set;

#define FD_SET(n, p) ((p)->bits[(n) >> 5] |= (1UL << ((n) & 31)))
#define FD_CLR(n, p) ((p)->bits[(n) >> 5] &= ~(1UL << ((n) & 31)))
#define FD_ISSET(n, p) (((p)->bits[(n) >> 5] & (1UL << ((n) & 31))) != 0)
#define FD_ZERO(p) do { unsigned _i; for (_i = 0; _i < (sizeof((p)->bits)/sizeof((p)->bits[0])); _i++) (p)->bits[_i] = 0; } while (0)

struct timeval;

int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int mega_select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

#endif

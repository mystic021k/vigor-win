#ifndef VIGOR_SYS_TIME_H
#define VIGOR_SYS_TIME_H

struct timeval {
	long tv_sec;
	long tv_usec;
};

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *, void *);

#include <sys/select.h>

#endif

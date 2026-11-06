#ifndef VIGOR_SYS_UIO_H
#define VIGOR_SYS_UIO_H

#include <sys/types.h>

struct iovec {
	void *iov_base;
	size_t iov_len;
};

int writev(int, const struct iovec *, int);
int readv(int, const struct iovec *, int);

#endif

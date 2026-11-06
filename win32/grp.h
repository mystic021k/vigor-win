#ifndef VIGOR_GRP_H
#define VIGOR_GRP_H

#include <sys/types.h>

struct group {
	char *gr_name;
	gid_t gr_gid;
};

struct group *getgrnam(const char *);

#endif

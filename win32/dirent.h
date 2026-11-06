#ifndef VIGOR_DIRENT_H
#define VIGOR_DIRENT_H

typedef struct DIR DIR;

struct dirent {
	char d_name[260];
};

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);

#endif

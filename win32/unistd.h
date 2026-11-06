#ifndef VIGOR_UNISTD_H
#define VIGOR_UNISTD_H

#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct timeval;

int getpid(void);

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

#ifndef F_OK
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
#endif

typedef unsigned long sigset_t;

struct sigaction {
	void (*sa_handler)(int);
	sigset_t sa_mask;
	int sa_flags;
};

#define SA_RESTART 1
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

int sigaction(int, const struct sigaction *, struct sigaction *);
int sigemptyset(sigset_t *);
int sigfillset(sigset_t *);
int sigaddset(sigset_t *, int);
int sigprocmask(int, const sigset_t *, sigset_t *);

int fsync(int);
int ftruncate(int, long);
int gettimeofday(struct timeval *, void *);

int pipe(int[2]);
int fork(void);
int vfork(void);
int execl(const char *, const char *, ...);
pid_t waitpid(pid_t, int *, int);
int kill(pid_t, int);

int fcntl(int, int, ...);
int flock(int, int);

uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setsid(void);
int chown(const char *, uid_t, gid_t);
int grantpt(int);
int unlockpt(int);
char *ptsname(int);
char *ttyname(int);
int gethostname(char *, int);

#include <sys/uio.h>

#define mkdir(path, mode) _mkdir(path)

int getopt(int, char * const *, const char *);
extern int opterr, optind, optopt, optreset;
extern char *optarg;

void win32_exit(int);
#define _exit win32_exit

#endif

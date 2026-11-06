#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <direct.h>
#include <sys/stat.h>

#include "dirent.h"
#include "grp.h"
#include "pwd.h"
#include "termios.h"
#include "unistd.h"
#include "sys/file.h"
#include "sys/ioctl.h"
#include "sys/time.h"
#include "sys/wait.h"

#undef _exit
#undef setvbuf

int
vigor_setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
	if (size == 0)
		size = BUFSIZ;
	return (setvbuf(stream, buffer, mode, size));
}
#undef close
#undef dup
#undef dup2
#undef read
#undef write
#undef open
#undef fileno
#undef isatty
#undef mkdir
#undef execl
#undef getpid

static volatile int in_child;
static jmp_buf child_jmp;
static volatile int fork_returned_pid;
static int child_std[3];

#define MAX_KIDS 32
struct kid {
	int used;
	DWORD pid;
	HANDLE process;
	int has_proc;
	int status;
};
static struct kid kids[MAX_KIDS];
static DWORD fake_pid = 100000;

static void
ensure_init(void)
{
	static int once;
	HANDLE out;
	DWORD mode;
	char tmp[MAX_PATH];
	char env[MAX_PATH + 16];
	size_t n;

	if (once)
		return;
	once = 1;
	_fmode = _O_BINARY;
	_setmode(0, _O_BINARY);
	_setmode(1, _O_BINARY);
	_setmode(2, _O_BINARY);
	out = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleMode(out, &mode))
		SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
	if (getenv("TMPDIR") == NULL && GetTempPathA(sizeof(tmp), tmp)) {
		n = strlen(tmp);
		while (n > 0 && (tmp[n - 1] == '\\' || tmp[n - 1] == '/'))
			tmp[--n] = '\0';
		_snprintf(env, sizeof(env), "TMPDIR=%s", tmp);
		_putenv(env);
	}
}

int
getpid(void)
{
	return ((int)GetCurrentProcessId());
}

int
fsync(int fd)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE)
		return (-1);
	return (FlushFileBuffers(h) ? 0 : -1);
}

int
ftruncate(int fd, long len)
{
	return (_chsize_s(fd, len) == 0 ? 0 : -1);
}

int
gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;
	ULARGE_INTEGER u;

	(void)tz;
	ensure_init();
	if (tv == NULL)
		return (0);
	GetSystemTimeAsFileTime(&ft);
	u.LowPart = ft.dwLowDateTime;
	u.HighPart = ft.dwHighDateTime;
	u.QuadPart -= 116444736000000000ULL;
	tv->tv_sec = (long)(u.QuadPart / 10000000ULL);
	tv->tv_usec = (long)((u.QuadPart % 10000000ULL) / 10ULL);
	return (0);
}

int
fcntl(int fd, int cmd, ...)
{
	(void)fd;
	if (cmd == F_GETFD || cmd == F_SETFD || cmd == F_GETFL || cmd == F_SETFL)
		return (0);
	errno = EINVAL;
	return (-1);
}

int
flock(int fd, int op)
{
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	OVERLAPPED ov;
	DWORD flags = 0;

	if (h == INVALID_HANDLE_VALUE)
		return (-1);
	memset(&ov, 0, sizeof(ov));
	if (op & LOCK_UN) {
		UnlockFile(h, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
		return (0);
	}
	if (op & LOCK_NB)
		flags |= LOCKFILE_FAIL_IMMEDIATELY;
	if (op & LOCK_EX)
		flags |= LOCKFILE_EXCLUSIVE_LOCK;
	if (!LockFileEx(h, flags, 0, 0xFFFFFFFF, 0xFFFFFFFF, &ov)) {
		errno = (GetLastError() == ERROR_LOCK_VIOLATION) ? EAGAIN : EIO;
		return (-1);
	}
	return (0);
}

int
close(int fd)
{
	if (in_child)
		return (0);
	return (_close(fd));
}

int
dup2(int oldfd, int newfd)
{
	if (in_child && newfd >= 0 && newfd < 3) {
		child_std[newfd] = oldfd;
		return (newfd);
	}
	return (_dup2(oldfd, newfd));
}

int
pipe(int fds[2])
{
	SECURITY_ATTRIBUTES sa;
	HANDLE rd, wr;

	ensure_init();
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&rd, &wr, &sa, 1 << 20)) {
		errno = EMFILE;
		return (-1);
	}
	fds[0] = _open_osfhandle((intptr_t)rd, _O_RDONLY | _O_BINARY);
	fds[1] = _open_osfhandle((intptr_t)wr, _O_WRONLY | _O_BINARY);
	if (fds[0] < 0 || fds[1] < 0)
		return (-1);
	return (0);
}

static struct kid *
alloc_kid(void)
{
	int i;
	for (i = 0; i < MAX_KIDS; i++) {
		if (!kids[i].used) {
			memset(&kids[i], 0, sizeof(kids[i]));
			kids[i].used = 1;
			return (&kids[i]);
		}
	}
	return (NULL);
}

static struct kid *
find_kid(DWORD pid)
{
	int i;
	for (i = 0; i < MAX_KIDS; i++)
		if (kids[i].used && kids[i].pid == pid)
			return (&kids[i]);
	return (NULL);
}

static int
do_fork(void)
{
	child_std[0] = 0;
	child_std[1] = 1;
	child_std[2] = 2;
	if (setjmp(child_jmp) == 0) {
		in_child = 1;
		return (0);
	}
	in_child = 0;
	return (fork_returned_pid);
}

int
fork(void)
{
	return (do_fork());
}

int
vfork(void)
{
	return (do_fork());
}

static HANDLE
inheritable(int fd)
{
	HANDLE h, dup;

	h = (HANDLE)_get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE || h == NULL)
		return (INVALID_HANDLE_VALUE);
	if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(),
	    &dup, 0, TRUE, DUPLICATE_SAME_ACCESS))
		return (h);
	return (dup);
}

static void
append_arg(char *cmd, size_t cap, const char *arg)
{
	size_t n = strlen(cmd);
	size_t i;
	int quote;

	if (n && n + 1 < cap)
		cmd[n++] = ' ';
	quote = (arg[0] == '\0');
	for (i = 0; arg[i]; i++)
		if (arg[i] == ' ' || arg[i] == '\t' || arg[i] == '"')
			quote = 1;
	if (quote && n + 1 < cap)
		cmd[n++] = '"';
	for (i = 0; arg[i] && n + 2 < cap; i++) {
		if (arg[i] == '"')
			cmd[n++] = '\\';
		cmd[n++] = arg[i];
	}
	if (quote && n + 1 < cap)
		cmd[n++] = '"';
	cmd[n] = '\0';
}

int
execl(const char *path, const char *arg0, ...)
{
	const char *args[64];
	int nargs = 0;
	va_list ap;
	char *cmdline;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	struct kid *slot;
	int interactive, i;
	DWORD flags;
	BOOL ok;
	const char *base;

	args[nargs++] = arg0;
	va_start(ap, arg0);
	for (;;) {
		const char *a = va_arg(ap, const char *);
		if (a == NULL || nargs >= 63)
			break;
		args[nargs++] = a;
	}
	va_end(ap);
	args[nargs] = NULL;

	base = strrchr(path, '\\');
	if (base == NULL)
		base = strrchr(path, '/');
	base = base ? base + 1 : path;
	if (_stricmp(base, "cmd.exe") == 0) {
		for (i = 1; i < nargs; i++)
			if (strcmp(args[i], "-c") == 0)
				args[i] = "/c";
	}

	cmdline = (char *)malloc(32768);
	if (cmdline == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	cmdline[0] = '\0';
	append_arg(cmdline, 32768, path);
	for (i = 1; i < nargs; i++)
		append_arg(cmdline, 32768, args[i]);

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	interactive = (child_std[0] == 0 && child_std[1] == 1 && child_std[2] == 2);
	if (!interactive) {
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = inheritable(child_std[0]);
		si.hStdOutput = inheritable(child_std[1]);
		si.hStdError = inheritable(child_std[2]);
		flags = CREATE_NO_WINDOW;
	} else
		flags = CREATE_NEW_CONSOLE;

	memset(&pi, 0, sizeof(pi));
	ok = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
	free(cmdline);
	if (!interactive) {
		if (si.hStdInput != INVALID_HANDLE_VALUE && si.hStdInput != (HANDLE)_get_osfhandle(child_std[0]))
			CloseHandle(si.hStdInput);
		if (si.hStdOutput != INVALID_HANDLE_VALUE && si.hStdOutput != (HANDLE)_get_osfhandle(child_std[1]))
			CloseHandle(si.hStdOutput);
		if (si.hStdError != INVALID_HANDLE_VALUE && si.hStdError != (HANDLE)_get_osfhandle(child_std[2]))
			CloseHandle(si.hStdError);
	}
	if (!ok) {
		errno = ENOENT;
		return (-1);
	}
	slot = alloc_kid();
	if (slot == NULL) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		errno = EAGAIN;
		return (-1);
	}
	slot->pid = pi.dwProcessId;
	slot->process = pi.hProcess;
	slot->has_proc = 1;
	CloseHandle(pi.hThread);
	fork_returned_pid = (int)pi.dwProcessId;
	if (in_child)
		longjmp(child_jmp, 1);
	return (-1);
}

void
win32_exit(int status)
{
	struct kid *slot;

	if (in_child) {
		slot = alloc_kid();
		if (slot != NULL) {
			slot->pid = fake_pid++;
			slot->has_proc = 0;
			slot->status = (status & 0xff) << 8;
			fork_returned_pid = (int)slot->pid;
		} else
			fork_returned_pid = -1;
		in_child = 0;
		longjmp(child_jmp, 1);
	}
	_exit(status);
}

pid_t
waitpid(pid_t pid, int *status, int options)
{
	struct kid *slot;
	DWORD code;

	slot = find_kid((DWORD)pid);
	if (slot == NULL) {
		errno = ECHILD;
		return (-1);
	}
	if (slot->has_proc) {
		if (options & WNOHANG) {
			if (WaitForSingleObject(slot->process, 0) == WAIT_TIMEOUT)
				return (0);
		} else
			WaitForSingleObject(slot->process, INFINITE);
		code = 1;
		GetExitCodeProcess(slot->process, &code);
		CloseHandle(slot->process);
		slot->has_proc = 0;
		slot->status = (int)((code & 0xff) << 8);
	}
	if (status != NULL)
		*status = slot->status;
	slot->used = 0;
	return (pid);
}

int
kill(pid_t pid, int sig)
{
	HANDLE h;

	if (sig == 0)
		return (0);
	if (pid > 0 && pid != getpid()) {
		h = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
		if (h == NULL) {
			errno = ESRCH;
			return (-1);
		}
		TerminateProcess(h, 1);
		CloseHandle(h);
		return (0);
	}
	if (sig == SIGINT)
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
	return (0);
}

static struct sigaction saved_act[32];

int
sigaction(int sig, const struct sigaction *act, struct sigaction *old)
{
	if (sig < 0 || sig >= 32) {
		errno = EINVAL;
		return (-1);
	}
	if (old != NULL)
		*old = saved_act[sig];
	if (act != NULL) {
		saved_act[sig] = *act;
		if (sig == SIGINT || sig == SIGTERM || sig == SIGABRT || sig == SIGBREAK)
			signal(sig, act->sa_handler);
	}
	return (0);
}

int
sigemptyset(sigset_t *set)
{
	*set = 0;
	return (0);
}

int
sigfillset(sigset_t *set)
{
	*set = ~(sigset_t)0;
	return (0);
}

int
sigaddset(sigset_t *set, int sig)
{
	if (sig < 0 || sig >= 32)
		return (-1);
	*set |= (1UL << sig);
	return (0);
}

int
sigprocmask(int how, const sigset_t *set, sigset_t *old)
{
	(void)how;
	(void)set;
	if (old != NULL)
		*old = 0;
	return (0);
}

uid_t getuid(void) { return (0); }
uid_t geteuid(void) { return (0); }
gid_t getgid(void) { return (0); }
gid_t getegid(void) { return (0); }
int setsid(void) { return (0); }

int
chown(const char *path, uid_t uid, gid_t gid)
{
	(void)path;
	(void)uid;
	(void)gid;
	return (0);
}

int
grantpt(int fd)
{
	(void)fd;
	errno = EINVAL;
	return (-1);
}

int
unlockpt(int fd)
{
	(void)fd;
	errno = EINVAL;
	return (-1);
}

char *
ttyname(int fd)
{
	(void)fd;
	return ("CON");
}

char *
ptsname(int fd)
{
	(void)fd;
	errno = EINVAL;
	return (NULL);
}

int
gethostname(char *name, int len)
{
	DWORD n = (DWORD)len;
	if (!GetComputerNameA(name, &n))
		return (-1);
	return (0);
}

int
setenv(const char *name, const char *value, int overwrite)
{
	char *buf;
	if (!overwrite && getenv(name) != NULL)
		return (0);
	buf = (char *)malloc(strlen(name) + strlen(value) + 2);
	if (buf == NULL)
		return (-1);
	sprintf(buf, "%s=%s", name, value);
	if (_putenv(buf) != 0)
		return (-1);
	return (0);
}

int
unsetenv(const char *name)
{
	char *buf = (char *)malloc(strlen(name) + 2);
	if (buf == NULL)
		return (-1);
	sprintf(buf, "%s=", name);
	_putenv(buf);
	return (0);
}

int
mkstemp(char *path)
{
	char *xs;
	int fd, tries;
	unsigned n;

	ensure_init();
	xs = strstr(path, "XXXXXX");
	if (xs == NULL) {
		errno = EINVAL;
		return (-1);
	}
	n = (unsigned)GetCurrentProcessId() ^ (unsigned)GetTickCount();
	for (tries = 0; tries < 100; tries++) {
		sprintf(xs, "%06u", (n + (unsigned)tries) % 1000000);
		fd = _open(path, _O_RDWR | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
		if (fd >= 0)
			return (fd);
	}
	errno = EEXIST;
	return (-1);
}

int
ioctl(int fd, unsigned long request, ...)
{
	va_list ap;
	struct winsize *ws;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE h;

	va_start(ap, request);
	if (request == TIOCGWINSZ) {
		ws = va_arg(ap, struct winsize *);
		va_end(ap);
		h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!GetConsoleScreenBufferInfo(h, &info)) {
			errno = ENOTTY;
			return (-1);
		}
		ws->ws_col = (unsigned short)(info.srWindow.Right - info.srWindow.Left + 1);
		ws->ws_row = (unsigned short)(info.srWindow.Bottom - info.srWindow.Top + 1);
		ws->ws_xpixel = 0;
		ws->ws_ypixel = 0;
		if (ws->ws_col == 0)
			ws->ws_col = 80;
		if (ws->ws_row == 0)
			ws->ws_row = 24;
		(void)fd;
		return (0);
	}
	va_end(ap);
	errno = EINVAL;
	return (-1);
}

static DWORD saved_in_mode, saved_out_mode;
static int have_in_mode, have_out_mode;

int
tcgetattr(int fd, struct termios *t)
{
	HANDLE h;
	DWORD mode;

	(void)fd;
	ensure_init();
	memset(t, 0, sizeof(*t));
	t->c_iflag = ICRNL | IXON;
	t->c_oflag = OPOST | ONLCR;
	t->c_lflag = ICANON | ECHO | ISIG;
	t->c_cc[VERASE] = 0x7f;
	t->c_cc[VKILL] = 0x15;
	t->c_cc[VEOF] = 4;
	t->c_cc[VINTR] = 3;
	t->c_cc[VMIN] = 1;
	t->c_cc[VTIME] = 0;
	h = GetStdHandle(STD_INPUT_HANDLE);
	if (GetConsoleMode(h, &mode)) {
		saved_in_mode = mode;
		have_in_mode = 1;
	}
	h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleMode(h, &mode)) {
		saved_out_mode = mode;
		have_out_mode = 1;
	}
	return (0);
}

int
tcsetattr(int fd, int actions, const struct termios *t)
{
	HANDLE in, out;
	DWORD inmode, outmode;

	(void)fd;
	(void)actions;
	ensure_init();
	in = GetStdHandle(STD_INPUT_HANDLE);
	out = GetStdHandle(STD_OUTPUT_HANDLE);
	if ((t->c_lflag & ICANON) && have_in_mode)
		inmode = saved_in_mode;
	else {
		inmode = ENABLE_VIRTUAL_TERMINAL_INPUT;
		if (t->c_lflag & ISIG)
			inmode |= ENABLE_PROCESSED_INPUT;
	}
	outmode = ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
	if (have_out_mode)
		outmode |= saved_out_mode & (ENABLE_PROCESSED_OUTPUT);
	SetConsoleMode(in, inmode);
	SetConsoleMode(out, outmode);
	return (0);
}

speed_t cfgetospeed(const struct termios *t) { (void)t; return (B9600); }
speed_t cfgetispeed(const struct termios *t) { (void)t; return (B9600); }
int cfsetospeed(struct termios *t, speed_t s) { t->c_ospeed = s; return (0); }
int cfsetispeed(struct termios *t, speed_t s) { t->c_ispeed = s; return (0); }

static int
fd_readable(int fd)
{
	HANDLE h;
	DWORD avail = 0, events = 0, type;

	h = (HANDLE)_get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE || h == NULL)
		return (0);
	type = GetFileType(h);
	if (type == FILE_TYPE_CHAR) {
		if (GetNumberOfConsoleInputEvents(h, &events) && events > 0)
			return (1);
		return (0);
	}
	if (type == FILE_TYPE_PIPE) {
		if (!PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL))
			return (1);
		return (avail > 0);
	}
	return (1);
}

void vigor_pump(void);
void vigor_on_select(void);

int
select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	fd_set ready;
	DWORD waited = 0, limit;
	int fd, n, any;

	(void)writefds;
	(void)exceptfds;
	ensure_init();
	vigor_pump();
	if (timeout == NULL)
		limit = 0xFFFFFFFF;
	else
		limit = (DWORD)(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);

	for (;;) {
		FD_ZERO(&ready);
		n = 0;
		any = 0;
		for (fd = 0; fd < nfds && fd < FD_SETSIZE; fd++) {
			if (readfds && FD_ISSET(fd, readfds)) {
				any = 1;
				if (fd_readable(fd)) {
					FD_SET(fd, &ready);
					n++;
				}
			}
		}
		if (!any)
			return (0);
		if (n > 0) {
			if (readfds)
				*readfds = ready;
			return (n);
		}
		if (waited >= limit && timeout != NULL)
			return (0);
		Sleep(20);
		waited += 20;
		vigor_pump();
	}
}

int
mega_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	vigor_on_select();
	return (select(nfds, readfds, writefds, exceptfds, timeout));
}

struct DIR {
	HANDLE h;
	WIN32_FIND_DATAA data;
	int first;
	struct dirent ent;
};

DIR *
opendir(const char *name)
{
	DIR *d;
	char pattern[MAX_PATH];
	size_t n;

	d = (DIR *)malloc(sizeof(*d));
	if (d == NULL)
		return (NULL);
	n = strlen(name);
	if (n + 3 >= sizeof(pattern)) {
		free(d);
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(pattern, name, n + 1);
	if (n > 0 && pattern[n - 1] != '\\' && pattern[n - 1] != '/')
		strcat(pattern, "\\*");
	else
		strcat(pattern, "*");
	d->h = FindFirstFileA(pattern, &d->data);
	if (d->h == INVALID_HANDLE_VALUE) {
		free(d);
		errno = ENOENT;
		return (NULL);
	}
	d->first = 1;
	return (d);
}

struct dirent *
readdir(DIR *d)
{
	if (d == NULL)
		return (NULL);
	if (!d->first) {
		if (!FindNextFileA(d->h, &d->data))
			return (NULL);
	}
	d->first = 0;
	strncpy(d->ent.d_name, d->data.cFileName, sizeof(d->ent.d_name) - 1);
	d->ent.d_name[sizeof(d->ent.d_name) - 1] = '\0';
	return (&d->ent);
}

int
closedir(DIR *d)
{
	if (d == NULL)
		return (-1);
	FindClose(d->h);
	free(d);
	return (0);
}

static struct passwd pw;
static char pw_name[256];
static char pw_dir[MAX_PATH];

struct passwd *
getpwuid(uid_t uid)
{
	DWORD n = sizeof(pw_name);

	(void)uid;
	if (!GetEnvironmentVariableA("USERNAME", pw_name, n))
		strcpy(pw_name, "user");
	if (!GetEnvironmentVariableA("USERPROFILE", pw_dir, sizeof(pw_dir)))
		strcpy(pw_dir, ".");
	pw.pw_name = pw_name;
	pw.pw_dir = pw_dir;
	pw.pw_uid = 0;
	pw.pw_gid = 0;
	return (&pw);
}

struct passwd *
getpwnam(const char *name)
{
	(void)name;
	return (getpwuid(0));
}

struct group *
getgrnam(const char *name)
{
	(void)name;
	return (NULL);
}

int
writev(int fd, const struct iovec *iov, int iovcnt)
{
	int i, total = 0;

	for (i = 0; i < iovcnt; i++) {
		int n = _write(fd, iov[i].iov_base, (unsigned)iov[i].iov_len);
		if (n < 0)
			return (-1);
		total += n;
		if ((size_t)n < iov[i].iov_len)
			break;
	}
	return (total);
}

int
readv(int fd, const struct iovec *iov, int iovcnt)
{
	int i, total = 0;

	for (i = 0; i < iovcnt; i++) {
		int n = _read(fd, iov[i].iov_base, (unsigned)iov[i].iov_len);
		if (n < 0)
			return (-1);
		total += n;
		if (n == 0 || (size_t)n < iov[i].iov_len)
			break;
	}
	return (total);
}

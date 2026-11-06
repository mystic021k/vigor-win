# Terminal nvi / vigor.exe, built with the MSVC compiler already on this machine.
# Run win32\build.bat from a normal command prompt (it loads the VS 2026 environment).

ROOT = ..
CC = cl
CFLAGS = /nologo /TC /W1 /O1 /c /FIforced.h /I. /I$(ROOT)\include /I$(ROOT)\curses /I$(ROOT)\db\include /I$(ROOT)\regex

all:
	if not exist obj\curses mkdir obj\curses
	if not exist obj\cl mkdir obj\cl
	if not exist obj\common mkdir obj\common
	if not exist obj\ex mkdir obj\ex
	if not exist obj\vi mkdir obj\vi
	if not exist obj\db mkdir obj\db
	if not exist obj\regex mkdir obj\regex
	if not exist obj\clib mkdir obj\clib
	if not exist obj\win32 mkdir obj\win32
	for %%F in ($(ROOT)\curses\*.c) do $(CC) $(CFLAGS) /D_CURSES_PRIVATE /Foobj\curses\ %%F || exit /b 1
	for %%F in ($(ROOT)\cl\*.c) do $(CC) $(CFLAGS) /Foobj\cl\ %%F || exit /b 1
	for %%F in ($(ROOT)\common\*.c) do $(CC) $(CFLAGS) /Foobj\common\ %%F || exit /b 1
	for %%F in ($(ROOT)\ex\*.c) do $(CC) $(CFLAGS) /Foobj\ex\ %%F || exit /b 1
	for %%F in ($(ROOT)\vi\*.c) do $(CC) $(CFLAGS) /Foobj\vi\ %%F || exit /b 1
	for %%F in ($(ROOT)\db\db\*.c $(ROOT)\db\mpool\*.c $(ROOT)\db\btree\*.c $(ROOT)\db\recno\*.c) do $(CC) $(CFLAGS) /D__DBINTERFACE_PRIVATE /Foobj\db\ %%F || exit /b 1
	for %%F in (regcomp.c regerror.c regexec.c regfree.c) do $(CC) $(CFLAGS) /D__REGEX_PRIVATE /Foobj\regex\ $(ROOT)\regex\%%F || exit /b 1
	for %%F in (getopt.c strsep.c mmap.c) do $(CC) $(CFLAGS) /Foobj\clib\ $(ROOT)\clib\%%F || exit /b 1
	for %%F in (posix.c termcap.c vigor_win.c) do $(CC) $(CFLAGS) /Foobj\win32\ %%F || exit /b 1
	dir /s /b obj\*.obj > objs.rsp
	link /nologo /OUT:vigor.exe @objs.rsp user32.lib gdi32.lib

clean:
	if exist obj rmdir /s /q obj
	if exist vigor.exe del /q vigor.exe
	if exist objs.rsp del /q objs.rsp
	if exist vigor.pdb del /q vigor.pdb
	if exist vigor.ilk del /q vigor.ilk
	if exist vigor.exp del /q vigor.exp

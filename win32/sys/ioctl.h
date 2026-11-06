#ifndef VIGOR_SYS_IOCTL_H
#define VIGOR_SYS_IOCTL_H

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

#define TIOCGWINSZ 0x5401
#define TIOCSWINSZ 0x5402
#define TIOCSCTTY  0x5403

int ioctl(int, unsigned long, ...);

#endif

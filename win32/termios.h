#ifndef VIGOR_TERMIOS_H
#define VIGOR_TERMIOS_H

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

#define VEOF 0
#define VEOL 1
#define VERASE 2
#define VKILL 3
#define VINTR 4
#define VQUIT 5
#define VSUSP 6
#define VSTART 7
#define VSTOP 8
#define VMIN 9
#define VTIME 10
#define VWERASE 11
#define VDSUSP 12
#define VDISCARD 13
#define VLNEXT 14
#define VSTATUS 15
#define VEOL2 16
#define VREPRINT 17

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
	speed_t c_ispeed;
	speed_t c_ospeed;
};

#define BRKINT  0x0001
#define ICRNL   0x0002
#define IGNBRK  0x0004
#define IGNCR   0x0008
#define INLCR   0x0010
#define ISTRIP  0x0020
#define IXON    0x0040
#define IXOFF   0x0080
#define PARMRK  0x0100

#define OPOST   0x0001
#define ONLCR   0x0002
#define OXTABS  0x0004

#define CSIZE   0x0030
#define CS8     0x0030
#define PARENB  0x0040

#define ECHO    0x0001
#define ECHOE   0x0002
#define ECHOK   0x0004
#define ECHONL  0x0008
#define ICANON  0x0010
#define IEXTEN  0x0020
#define ISIG    0x0040
#define NOFLSH  0x0080
#define TOSTOP  0x0100

#define B0     0
#define B50    50
#define B75    75
#define B110   110
#define B134   134
#define B150   150
#define B200   200
#define B300   300
#define B600   600
#define B1200  1200
#define B1800  1800
#define B2400  2400
#define B4800  4800
#define B9600  9600
#define B19200 19200
#define B38400 38400

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2
#define TCSASOFT  0x80000000

int tcgetattr(int, struct termios *);
int tcsetattr(int, int, const struct termios *);
speed_t cfgetospeed(const struct termios *);
speed_t cfgetispeed(const struct termios *);
int cfsetospeed(struct termios *, speed_t);
int cfsetispeed(struct termios *, speed_t);

#endif

#include "termios.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct cap {
	char name[3];
	const char *str;
	int num;
	int flag;
	int kind; /* 0 flag, 1 number, 2 string */
};

/* Enough of an ANSI/VT terminal for the bundled curses. */
static struct cap caps[] = {
	{"am", 0, 0, 1, 0},
	{"bs", 0, 0, 1, 0},
	{"ms", 0, 0, 1, 0},
	{"xn", 0, 0, 1, 0},
	{"co", 0, 80, 0, 1},
	{"li", 0, 24, 0, 1},
	{"sg", 0, 0, 0, 1},
	{"ug", 0, 0, 0, 1},
	{"it", 0, 8, 0, 1},
	{"cl", "\033[H\033[2J", 0, 0, 2},
	{"cd", "\033[J", 0, 0, 2},
	{"ce", "\033[K", 0, 0, 2},
	{"cm", "\033[%i%d;%dH", 0, 0, 2},
	{"ho", "\033[H", 0, 0, 2},
	{"cr", "\r", 0, 0, 2},
	{"nd", "\033[C", 0, 0, 2},
	{"up", "\033[A", 0, 0, 2},
	{"do", "\n", 0, 0, 2},
	{"le", "\033[D", 0, 0, 2},
	{"al", "\033[L", 0, 0, 2},
	{"dl", "\033[M", 0, 0, 2},
	{"AL", "\033[%dL", 0, 0, 2},
	{"DL", "\033[%dM", 0, 0, 2},
	{"sf", "\n", 0, 0, 2},
	{"sr", "\033M", 0, 0, 2},
	{"im", "\033[4h", 0, 0, 2},
	{"ei", "\033[4l", 0, 0, 2},
	{"ic", "\033[@", 0, 0, 2},
	{"dc", "\033[P", 0, 0, 2},
	{"so", "\033[7m", 0, 0, 2},
	{"se", "\033[27m", 0, 0, 2},
	{"us", "\033[4m", 0, 0, 2},
	{"ue", "\033[24m", 0, 0, 2},
	{"md", "\033[1m", 0, 0, 2},
	{"me", "\033[m", 0, 0, 2},
	{"ti", "\033[?1049h", 0, 0, 2},
	{"te", "\033[?1049l", 0, 0, 2},
	{"ku", "\033[A", 0, 0, 2},
	{"kd", "\033[B", 0, 0, 2},
	{"kr", "\033[C", 0, 0, 2},
	{"kl", "\033[D", 0, 0, 2},
	{"kh", "\033[H", 0, 0, 2},
	{"k1", "\033OP", 0, 0, 2},
	{"k2", "\033OQ", 0, 0, 2},
	{"k3", "\033OR", 0, 0, 2},
	{"k4", "\033OS", 0, 0, 2},
	{"sc", "\0337", 0, 0, 2},
	{"rc", "\0338", 0, 0, 2},
	{"vb", "\033[?5h\033[?5l", 0, 0, 2},
	{"vs", "", 0, 0, 2},
	{"ve", "", 0, 0, 2},
	{"ta", "\t", 0, 0, 2},
	{"bc", "\b", 0, 0, 2},
	{"UP", "\033[%dA", 0, 0, 2},
	{"DO", "\033[%dB", 0, 0, 2},
	{"LE", "\033[%dD", 0, 0, 2},
	{"RI", "\033[%dC", 0, 0, 2},
};

static struct cap *
findcap(const char *id)
{
	size_t i;

	if (id == NULL)
		return (NULL);
	for (i = 0; i < sizeof(caps) / sizeof(caps[0]); i++)
		if (caps[i].name[0] == id[0] && caps[i].name[1] == id[1])
			return (&caps[i]);
	return (NULL);
}

int
tgetent(char *bp, const char *name)
{
	(void)bp;
	(void)name;
	return (1);
}

int
tgetflag(const char *id)
{
	struct cap *c = findcap(id);
	return (c != NULL && c->kind == 0 && c->flag);
}

int
tgetnum(const char *id)
{
	struct cap *c = findcap(id);
	if (c == NULL || c->kind != 1)
		return (-1);
	return (c->num);
}

char *
tgetstr(const char *id, char **area)
{
	struct cap *c = findcap(id);
	char *out;
	size_t n;

	if (c == NULL || c->kind != 2 || c->str == NULL || area == NULL || *area == NULL)
		return (NULL);
	if (c->str[0] == '\0')
		return (NULL);
	n = strlen(c->str);
	out = *area;
	memcpy(out, c->str, n + 1);
	*area += n + 1;
	return (out);
}

static char *
putnum(char *dp, int v)
{
	char tmp[16];
	int i = 0, n;

	if (v < 0)
		v = 0;
	do {
		tmp[i++] = (char)('0' + (v % 10));
		v /= 10;
	} while (v && i < (int)sizeof(tmp));
	while (i > 0)
		*dp++ = tmp[--i];
	(void)n;
	return (dp);
}

char *
tgoto(const char *cap, int destcol, int destline)
{
	static char result[128];
	const char *cp;
	char *dp = result;
	int which, oncol, c;

	if (cap == NULL || cap[0] == '\0')
		return (NULL);
	cp = cap;
	which = destline;
	oncol = 0;
	while ((c = (unsigned char)*cp++) != 0) {
		if (dp >= result + sizeof(result) - 2)
			break;
		if (c != '%') {
			*dp++ = (char)c;
			continue;
		}
		switch (*cp++) {
		case 'd':
			dp = putnum(dp, which);
			goto flip;
		case '2':
		case '3':
			dp = putnum(dp, which);
			goto flip;
		case '.':
			*dp++ = (char)which;
			goto flip;
		case '+':
			which += (unsigned char)*cp++;
			*dp++ = (char)which;
flip:
			oncol = !oncol;
			which = oncol ? destcol : destline;
			break;
		case 'i':
			destcol++;
			destline++;
			which++;
			break;
		case '%':
			*dp++ = '%';
			break;
		case 'r': {
			int t = destcol;
			destcol = destline;
			destline = t;
			which = oncol ? destcol : destline;
			break;
		}
		case 'n':
			destcol ^= 0140;
			destline ^= 0140;
			which = oncol ? destcol : destline;
			break;
		default:
			break;
		}
	}
	*dp = '\0';
	return (result);
}

int
tputs(const char *str, int affcnt, int (*outc)(int))
{
	(void)affcnt;
	if (str == NULL || outc == NULL)
		return (0);
	if (*str >= '0' && *str <= '9') {
		while (*str >= '0' && *str <= '9')
			str++;
		if (*str == '.') {
			str++;
			while (*str >= '0' && *str <= '9')
				str++;
		}
		if (*str == '*')
			str++;
	}
	while (*str) {
		if (*str == '$' && str[1] == '<') {
			str += 2;
			while (*str && *str != '>')
				str++;
			if (*str == '>')
				str++;
			continue;
		}
		outc((unsigned char)*str++);
	}
	return (0);
}

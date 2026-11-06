/*-
 * Copyright (c) 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 *
 * See the LICENSE file for redistribution information.
 */

#include "config.h"

/* FIXME Most of these are unnecessary. */
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <bitstring.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include <termios.h>
#include <unistd.h>

#include "../common/common.h"
#include "vigor_extern.h"

#include "vigor_empty.xbm"
#include "vigor_rest.xbm"

#include "vigor_ent_0001.xbm"
#include "vigor_ent_0002.xbm"
#include "vigor_ent_0003.xbm"
#include "vigor_ent_0004.xbm"

#include "vigor_wav_0001.xbm"
#include "vigor_wav_0002.xbm"

#include "v2cp0000.xbm"
#include "v2cp0001.xbm"
#include "v2cp0002.xbm"
#include "v2cp0003.xbm"
#include "v2cp0004.xbm"

#include "vigor.tcl.h"
static void init_vigor_phrases __P((void));

/* Some platforms and/or versions of TCL may require this to be a void*
 * or char*.  I haven't heard of anybody who does, though, so tell me
 * if you do.
 *
 * I hate global variables as much as the next guy, but I make an
 * allowance in quick hacks.  Passing down the GS to all the hooks
 * that I use would represent a significant rewrite of nvi code.
 */
static Tcl_Interp *vigor_interp = NULL;

/*
 * The initialization sets up the interpreter, all the external stuff
 * that Vigor needs, and then calls the Tcl vigor_init.  That handles
 * the EULA and greeting.
 */
int
vigor_init(gp)
	GS *gp;
{
	tcl_init(gp);
	vigor_interp = gp->tcl_interp;

	/* We put Tk_Init in Vigor since the user probably isn't going
	 * to want tk's main window cluttering up the place.  I should
	 * use a wm_withdraw instead. */
	if (Tk_Init(vigor_interp) == TCL_ERROR) {
		fprintf(stderr, "%s\n", vigor_interp->result);		
		return (1);
	}

#if 0
	if (Tcl_EvalFile(vigor_interp, "vigor.tcl") == TCL_ERROR)
		return (1);
#endif
	if (Tcl_Eval(vigor_interp, vigor_tcl) == TCL_ERROR) {
		fprintf(stderr, "%s\n", vigor_interp->result);
		return (1);
	}

	/* I don't want to use a #define for the Tk_DefineBitmap calls
	 * because I don't remember how this differs between ANSI and K&R.
	 */
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_empty"),
			vigor_empty_bits, vigor_empty_width, vigor_empty_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_rest"),
			vigor_rest_bits, vigor_rest_width, vigor_rest_height);

	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_ent_0001"),
			vigor_ent_0001_bits, vigor_ent_0001_width, vigor_ent_0001_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_ent_0002"),
			vigor_ent_0002_bits, vigor_ent_0002_width, vigor_ent_0002_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_ent_0003"),
			vigor_ent_0003_bits, vigor_ent_0003_width, vigor_ent_0003_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_ent_0004"),
			vigor_ent_0004_bits, vigor_ent_0004_width, vigor_ent_0004_height);

	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_wav_0001"),
			vigor_wav_0001_bits, vigor_wav_0001_width, vigor_wav_0001_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("vigor_wav_0002"),
			vigor_wav_0002_bits, vigor_wav_0002_width, vigor_wav_0002_height);

	Tk_DefineBitmap(vigor_interp, Tk_GetUid("v2cp0000"),
			v2cp0000_bits, v2cp0000_width, v2cp0000_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("v2cp0001"),
			v2cp0001_bits, v2cp0001_width, v2cp0001_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("v2cp0002"),
			v2cp0002_bits, v2cp0002_width, v2cp0002_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("v2cp0003"),
			v2cp0003_bits, v2cp0003_width, v2cp0003_height);
	Tk_DefineBitmap(vigor_interp, Tk_GetUid("v2cp0004"),
			v2cp0004_bits, v2cp0004_width, v2cp0004_height);

	init_vigor_phrases();

	if (Tcl_Eval(vigor_interp, "vigor_init") == TCL_ERROR) {
		fprintf(stderr, "%s\n", vigor_interp->result);
	        return (1);
	}

	return (0);
}

/*
 * These are the hooks that constitute most of Vigor's functionality.
 * They should be liberally called throughout the Vigor code.  I wrote
 * the first version and included just a few hook calls, and never
 * really got around to adding more.  Contributions are welcome!
 */

void
vigor_comment(expr, gp)
	char *expr;
	GS *gp;
{
	Tcl_VarEval(vigor_interp, "vigor_comment ", expr, (char*)NULL);
}

void
vigor_hint(expr, gp)
	char *expr;
	GS *gp;
{
	Tcl_VarEval(vigor_interp, "vigor_hint ", expr, (char*)NULL);
}

void
vigor_confirm(expr, gp)
	char *expr;
	GS *gp;
{
	Tcl_VarEval(vigor_interp, "vigor_confirm ", expr, (char*)NULL);
}

void
vigor_maybe_confirm(prob, expr, gp)
	int prob;
	char *expr;
	GS *gp;
{
	char buf[20];
	sprintf(buf, " %i ", prob);
	Tcl_VarEval(vigor_interp, "vigor_maybe_confirm ", buf, expr, (char*)NULL);
}

void
vigor_wizard(wizname, task0, task1, task2, this_task, descr, gp)
	char *wizname;
	char *task0, *task1, *task2;
	int this_task;
	char *descr;
	GS *gp;
{
        char buf[1024];
	sprintf(buf, "vigor_wizard %s %s %s %s %i ", wizname,
		task0, task1, task2, this_task);
	Tcl_VarEval(vigor_interp, buf, descr, (char*)NULL);
}

/*
 * vigor_maybe_wizard: Maybe offer to give the user a wizard.
 *
 * prob: Probability, a percentage figure
 * wizname: The name of the "wizard series".  This should be something that
 *    fits in the sentence, "You are %s".
 * task0: One possible wizard.  This should fit into the sentence, "You are
 *    trying to %s".
 * task1, task2: Same
 * this_task: Which task corresponds to the wizard being invoked.  This is
 *    presently ignored; task0 should always be the wizard being invoked.
 * The above strings should be less than about 1000 bytes.
 * descr: The wizard text.
 */

void
vigor_maybe_wizard(prob, wizname, task0, task1, task2, this_task, descr, gp)
        int prob;
	char *wizname;
	char *task0, *task1, *task2;
	int this_task;
	char *descr;
	GS *gp;
{
        char buf[1024];
	sprintf(buf, "vigor_maybe_wizard %i %s %s %s %s %i ", prob, wizname,
		task0, task1, task2, this_task);
	Tcl_VarEval(vigor_interp, buf, descr, (char*)NULL);
}

/*
 * This section is responsible for handling key phrases that invoke
 * the wizards.  This is one of the few areas of Vigor code that is
 * primarily C-based, since it's faster and much easier for this than
 * Tcl would be, particularly since I'm a C kinda guy.
 *
 * We handle this by watching user input for a particular series of
 * keystrokes.  This has the advantage of being easy to implement.  It
 * has the disadvantage of requiring the entire phrase to be typed
 * at once, with no mistakes.
 *
 * FIXME This could be better handled in the abbreviation expansion code.
 * If Vigor ever goes production-quality, then that will be one change.
 */

/*
 * This struct contains the phrase to watch for, a pointer that indicates
 * the next character in that phrase, and the args to vigor_wizard.
 * Note that in the current implementation, a phrase should start with
 * a character that is not used elsewhere in the phrase.
 *
 * This would be better implemented as an NDFA.  This is inappropriate for
 * 12:15 AM silly-hacking when I want to get the next release out tonight.
 */
static struct vigor_phrase {
    char *phrase;
    char *cp;
    char *wizname;
    char *task0;
    char *task1;
    char *task2;
    int this_task;
    char *descr;
} vigor_phrases[] = {
    { "#!", NULL, "\"starting a program\"",
      "\"write a shell script\"","\"write a C program\"","\"write Tcl code\"",
      0, "\"You can use a shell script to give a series of commands that are "
      "implemented by the shell.  You should give commands in shell "
      "language.  The shell script should also tell the computer which "
      "shell it should be run under.\""
    },
    { "#include", NULL, "\"starting a program\"",
      "\"write a C program\"","\"write Tcl code\"","\"write a Lisp program\"",
      0, "\"You can use a C program to perform a variety of tasks that cannot "
      "be easily written by shell scripts.  Writing a C program requires "
      "careful attention to detail.  You will have to compile your C program "
      "before you can use it.\""
    },
    /* Yes, assuming that an open paren is necessarily a sign of Lisp code
     * is very annoying.  But if you think it's annoying to use Vigor, try
     * testing it sometime.
     */
    { "(", NULL, "\"starting a program\"",
      "\"write a Lisp program\"","\"write Tcl code\"",
      "\"write a shell script\"", 0,
      "\"You can write a Lisp program to perform most tasks you can with a C "
      "program.  Lisp programs also have the advantage that very few "
      "programmers know Lisp, so your employer will have to keep you on "
      "staff to maintain it.\""
    },
    { NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL }
};

void
vigor_gotchar(ch)
    char ch;
{
    struct vigor_phrase *pp;

    for (pp = vigor_phrases; pp->phrase; pp++)
	if (*pp->cp != ch)
	    pp->cp = pp->phrase;
	else if (!*++pp->cp)
	    vigor_wizard(pp->wizname, pp->task0, pp->task1, pp->task2,
			 pp->this_task, pp->descr);
}

static void
init_vigor_phrases()
{
    struct vigor_phrase *pp;

    for (pp = vigor_phrases; pp->phrase; pp++)
	pp->cp = pp->phrase;
}

/*
 * This section handles the select overrides.  In particular, we need
 * to replace select, so that Tk's timers can run.  This is used for
 * animation and the random popups.
 */

static int waiting_for_input;

static void
tk_file_ev(clientData, mask)
	ClientData clientData;
	int mask;
{
	waiting_for_input = 0;
}

static void
tk_timer_ev(clientData)
	ClientData clientData;
{
	waiting_for_input = 0;
}

int
mega_select(nfds, readfds, writefds, exceptfds, timeout)
	int nfds;
	fd_set *readfds, *writefds, *exceptfds;
	struct timeval *timeout;
{
	int fd;
	int mask;
	int ms;
        struct timeval notime;	/* FIXME should be static */
	Tk_TimerToken cookie;

	if (!vigor_interp) {
	    /* Vigor has not yet been initialized... Use normal
	     * select instead.
	     */
	    return select(nfds, readfds, writefds, exceptfds, timeout);
	}

	Tcl_Eval(vigor_interp, "vigor_randomness");
	/* Don't check the return code; we need to select anyway. */

	for (fd = 0; fd < nfds; fd++) {
		mask = 0;
		if (readfds && FD_ISSET(fd, readfds))
			mask |= TK_READABLE;
		if (writefds && FD_ISSET(fd, writefds))
			mask |= TK_WRITABLE;
		if (exceptfds && FD_ISSET(fd, exceptfds))
			mask |= TK_EXCEPTION;
		if (mask)
			Tk_CreateFileHandler(fd, mask, tk_file_ev,
					     (ClientData) NULL);
	}

	waiting_for_input = 1;
	if (timeout) {
		ms = timeout->tv_usec / 1000 + timeout->tv_sec * 1000;
		if (ms > 0) {
			/* A non-zero timeout */
			/* I don't know how often this happens. */
			cookie = Tk_CreateTimerHandler(ms, tk_timer_ev,
						       (ClientData) NULL);
			while (waiting_for_input)
				Tk_DoOneEvent(0);
			Tk_DeleteTimerHandler(cookie);
		} else {
			/* A zero timeout */
			while (Tk_DoOneEvent(TK_DONT_WAIT)
			       && waiting_for_input)
				/* nothing happens */;
		}
	} else {
		/* Indefinite timeout */
		while (waiting_for_input)
			Tk_DoOneEvent(0);
	}
	
	for (fd = 0; fd < nfds; fd++) {
		/* Is there a Tk_FlushFileHandlers?
		 * See Ousterhout's TCL book, p398, for why what I'm doing
		 * here is wrong. */
		Tk_DeleteFileHandler(fd);
	}

	/* Fortunately, we don't care about updating select's timeout
	 * (which some Unices don't do anyway) */
	notime.tv_usec = 0;
	notime.tv_sec = 0;
	return select(nfds, readfds, writefds, exceptfds, &notime);
}

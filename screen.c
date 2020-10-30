/*@@copyright@@*/


/*
 * Routines which deal with the characteristics of the terminal.
 * Uses termcap to be as terminal-independent as possible.
 */

#include "less.h"
#include "cmd.h"
#include "os_defs.h"
#include "pckeys.h"

#if !UNIX

#if defined(WIN32)
#include <windows.h>
#elif DOS_DJGPPC || DOS_BORLAND
#include <conio.h>
#if DOS_DJGPPC
#include <pc.h>
extern int fd0;
#endif
#endif

#else

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if HAVE_TERMIOS_H && HAVE_TERMIOS_FUNCS
#include <termios.h>
#else
#if HAVE_TERMIO_H
#include <termio.h>
#else
#if HAVE_SGSTAT_H
#include <sgstat.h>
#else
#include <sgtty.h>
#endif
#endif
#endif

#if HAVE_TERMCAP_H
#include <termcap.h>
#endif
#ifdef _OSK
#include <signal.h>
#endif
#if OS2
#include <sys/signal.h>
#include "pckeys.h"
#endif
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_SYS_PTEM_H
#include <sys/ptem.h>
#endif

#endif /* !UNIX */

/*
 * Check for broken termios package that forces you to manually
 * set the line discipline.
 */
#ifdef __ultrix__
#define MUST_SET_LINE_DISCIPLINE 1
#else
#define MUST_SET_LINE_DISCIPLINE 0
#endif

#if OS2
#define	DEFAULT_TERM		"ansi"
static char *windowid;
#else
#define	DEFAULT_TERM		"unknown"
#endif

#if DOS_MSC
static int videopages;
static long msec_loops;
static int flash_created = 0;
#define	SETCOLORS(fg,bg)	{ _settextcolor(fg); _setbkcolor(bg); }
#endif

#if DOS_BORLAND
static unsigned short *whitescreen;
static int flash_created = 0;
#endif
#if DOS_DJGPPC || DOS_BORLAND
#define _settextposition(y,x)   gotoxy(x,y)
#define _clearscreen(m)         clrscr()
#define _outtext(s)             cputs(s)
#define	SETCOLORS(fg,bg)	{ textcolor(fg); textbackground(bg); }
extern int sc_height;
#endif


#if defined(WIN32)
struct keyRecord
{
	int ascii;
	int scan;
} currentKey;

static int keyCount = 0;
static WORD curr_attr;
static int pending_scancode = 0;
static char x11mousebuf[] = "[M???";    /* Mouse report, after ESC */
static int x11mousePos, x11mouseCount;

static HANDLE con_out_save = INVALID_HANDLE_VALUE; /* previous console */
static HANDLE con_out_ours = INVALID_HANDLE_VALUE; /* our own */
HANDLE con_out = INVALID_HANDLE_VALUE;             /* current console */

extern int utf_mode;
extern int quitting;
static void win32_init_term();
static void win32_deinit_term();

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
#endif

#define FG_COLORS       (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define BG_COLORS       (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define	MAKEATTR(fg,bg)		((WORD)((fg)|((bg)<<4)))
#define SETCOLORS(fg,bg) { \
	curr_attr = MAKEATTR(fg,bg); \
	if (SetConsoleTextAttribute(con_out, curr_attr) == 0) \
			error("SETCOLORS failed", NULL_PARG); }
#endif

#if !UNIX
int nm_fg_color;		/* Color of normal text */
int nm_bg_color;
int bo_fg_color;		/* Color of bold text */
int bo_bg_color;
int ul_fg_color;		/* Color of underlined text */
int ul_bg_color;
int so_fg_color;		/* Color of standout text */
int so_bg_color;
int bl_fg_color;		/* Color of blinking text */
int bl_bg_color;
static int sy_fg_color;		/* Color of system text (before less) */
static int sy_bg_color;
int sgr_mode;		/* Honor ANSI sequences rather than using above */
#if defined(WIN32)
DWORD init_output_mode;		/* The initial console output mode */
bool vt_enabled;		/* Is virtual terminal processing available? */
#endif
#else

/*
 * Strings passed to tputs() to do various terminal functions.
 */
static char
	*sc_pad,		/* Pad string */
	*sc_home,		/* Cursor home */
	*sc_addline,		/* Add line, scroll down following lines */
	*sc_lower_left,		/* Cursor to last line, first column */
	*sc_return,		/* Cursor to beginning of current line */
	*sc_move,		/* General cursor positioning */
	*sc_clear,		/* Clear screen */
	*sc_eol_clear,		/* Clear to end of line */
	*sc_eos_clear,		/* Clear to end of screen */
	*sc_s_in,		/* Enter standout (highlighted) mode */
	*sc_s_out,		/* Exit standout mode */
	*sc_u_in,		/* Enter underline mode */
	*sc_u_out,		/* Exit underline mode */
	*sc_b_in,		/* Enter bold mode */
	*sc_b_out,		/* Exit bold mode */
	*sc_bl_in,		/* Enter blink mode */
	*sc_bl_out,		/* Exit blink mode */
	*sc_visual_bell,	/* Visual bell (flash screen) sequence */
	*sc_backspace,		/* Backspace cursor */
	*sc_s_keypad,		/* Start keypad mode */
	*sc_e_keypad,		/* End keypad mode */
	*sc_s_mousecap,		/* Start mouse capture mode */
	*sc_e_mousecap,		/* End mouse capture mode */
	*sc_init,		/* Startup terminal initialization */
	*sc_deinit;		/* Exit terminal de-initialization */
#endif

static int init_done = 0;

int auto_wrap;		/* Terminal does \r\n when write past margin */
int ignaw;		/* Terminal ignores \n immediately after wrap */
int erase_char;		/* The user's erase char */
int erase2_char;		/* The user's other erase char */
int kill_char;		/* The user's line-kill char */
int werase_char;		/* The user's word-erase char */
int sc_width, sc_height;	/* Height & width of screen */
int bo_s_width, bo_e_width;	/* Printing width of boldface seq */
int ul_s_width, ul_e_width;	/* Printing width of underline seq */
int so_s_width, so_e_width;	/* Printing width of standout seq */
int bl_s_width, bl_e_width;	/* Printing width of blink seq */
int above_mem, below_mem;	/* Memory retained above/below screen */
int can_goto_line;		/* Can move cursor to any line */
int clear_bg;		/* Clear fills with background color */
int missing_cap = 0;	/* Some capability is missing */
char *kent = NULL;	/* Keypad ENTER sequence */

static int attrmode = AT_NORMAL;
static int termcap_debug = -1;
extern int binattr;
extern int one_screen;

#if UNIX
static char *cheaper (char *t1, char *t2, char *def);
static void tmodes (char *incap, char *outcap, char **instr,
    char **outstr, char *def_instr, char *def_outstr, char **spp);
#endif

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 */
#if MUST_DEFINE_OSPEED
extern short ospeed;	/* Terminal output baud rate */
extern char PC;		/* Pad character */
#endif
#ifdef _OSK
short ospeed;
char PC_, *UP, *BC;
#endif

extern int quiet;		/* If VERY_QUIET, use visual bell for bell */
extern int no_back_scroll;
extern int swindow;
extern int no_init;
extern int no_keypad;
extern int sigs;
extern int wscroll;
extern int screen_trashed;
extern int top_scroll;
extern int quit_if_one_screen;
extern int oldbot;
extern int mousecap;
#if HILITE_SEARCH
extern int hilite_search;
#endif
#if defined(WIN32)
extern HANDLE tty;
extern DWORD console_mode;
#ifndef ENABLE_EXTENDED_FLAGS
#define ENABLE_EXTENDED_FLAGS 0x80
#define ENABLE_QUICK_EDIT_MODE 0x40
#endif
#else
extern int tty;
#endif

extern char *tgetstr();
extern char *tgoto();


/*
 * Change terminal to "raw mode", or restore to "normal" mode.
 * "Raw mode" means 
 *	1. An outstanding read will complete on receipt of a single keystroke.
 *	2. Input is not echoed.  
 *	3. On output, \n is mapped to \r\n.
 *	4. \t is NOT expanded into spaces.
 *	5. Signal-causing characters such as ctrl-C (interrupt),
 *	   etc. are NOT disabled.
 * It doesn't matter whether an input \n is mapped to \r, or vice versa.
 */
void raw_mode(int on)
{
	static int curr_on = 0;

	if (on == curr_on)
		return;
	erase2_char = '\b'; /* in case OS doesn't know about erase2 */
#if HAVE_TERMIOS_H && HAVE_TERMIOS_FUNCS
    {
	struct termios s;
	static struct termios save_term;
	static int saved_term = 0;

	if (on) 
	{
		/*
		 * Get terminal modes.
		 */
		tcgetattr(tty, &s);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		if (!saved_term)
		{
			save_term = s;
			saved_term = 1;
		}
#if HAVE_OSPEED
		switch (cfgetospeed(&s))
		{
#ifdef B0
		case B0: ospeed = 0; break;
#endif
#ifdef B50
		case B50: ospeed = 1; break;
#endif
#ifdef B75
		case B75: ospeed = 2; break;
#endif
#ifdef B110
		case B110: ospeed = 3; break;
#endif
#ifdef B134
		case B134: ospeed = 4; break;
#endif
#ifdef B150
		case B150: ospeed = 5; break;
#endif
#ifdef B200
		case B200: ospeed = 6; break;
#endif
#ifdef B300
		case B300: ospeed = 7; break;
#endif
#ifdef B600
		case B600: ospeed = 8; break;
#endif
#ifdef B1200
		case B1200: ospeed = 9; break;
#endif
#ifdef B1800
		case B1800: ospeed = 10; break;
#endif
#ifdef B2400
		case B2400: ospeed = 11; break;
#endif
#ifdef B4800
		case B4800: ospeed = 12; break;
#endif
#ifdef B9600
		case B9600: ospeed = 13; break;
#endif
#ifdef EXTA
		case EXTA: ospeed = 14; break;
#endif
#ifdef EXTB
		case EXTB: ospeed = 15; break;
#endif
#ifdef B57600
		case B57600: ospeed = 16; break;
#endif
#ifdef B115200
		case B115200: ospeed = 17; break;
#endif
		default: ;
		}
#endif
		erase_char = s.c_cc[VERASE];
#ifdef VERASE2
		erase2_char = s.c_cc[VERASE2];
#endif
		kill_char = s.c_cc[VKILL];
#ifdef VWERASE
		werase_char = s.c_cc[VWERASE];
#else
		werase_char = CONTROL('W');
#endif

		/*
		 * Set the modes to the way we want them.
		 */
		s.c_lflag &= ~(0
#ifdef ICANON
			| ICANON
#endif
#ifdef ECHO
			| ECHO
#endif
#ifdef ECHOE
			| ECHOE
#endif
#ifdef ECHOK
			| ECHOK
#endif
#if ECHONL
			| ECHONL
#endif
		);

		s.c_oflag |= (0
#ifdef OXTABS
			| OXTABS
#else
#ifdef TAB3
			| TAB3
#else
#ifdef XTABS
			| XTABS
#endif
#endif
#endif
#ifdef OPOST
			| OPOST
#endif
#ifdef ONLCR
			| ONLCR
#endif
		);

		s.c_oflag &= ~(0
#ifdef ONOEOT
			| ONOEOT
#endif
#ifdef OCRNL
			| OCRNL
#endif
#ifdef ONOCR
			| ONOCR
#endif
#ifdef ONLRET
			| ONLRET
#endif
		);
		s.c_cc[VMIN] = 1;
		s.c_cc[VTIME] = 0;
#ifdef VLNEXT
		s.c_cc[VLNEXT] = 0;
#endif
#ifdef VDSUSP
		s.c_cc[VDSUSP] = 0;
#endif
#if MUST_SET_LINE_DISCIPLINE
		/*
		 * System's termios is broken; need to explicitly 
		 * request TERMIODISC line discipline.
		 */
		s.c_line = TERMIODISC;
#endif
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
#if HAVE_FSYNC
	fsync(tty);
#endif
	tcsetattr(tty, TCSADRAIN, &s);
#if MUST_SET_LINE_DISCIPLINE
	if (!on)
	{
		/*
		 * Broken termios *ignores* any line discipline
		 * except TERMIODISC.  A different old line discipline
		 * is therefore not restored, yet.  Restore the old
		 * line discipline by hand.
		 */
		ioctl(tty, TIOCSETD, &save_term.c_line);
	}
#endif
    }
#elif defined(TCGETA)
    {
	struct termio s;
	static struct termio save_term;
	static int saved_term = 0;

	if (on)
	{
		/*
		 * Get terminal modes.
		 */
		ioctl(tty, TCGETA, &s);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		if (!saved_term)
		{
			save_term = s;
			saved_term = 1;
		}
#if HAVE_OSPEED
		ospeed = s.c_cflag & CBAUD;
#endif
		erase_char = s.c_cc[VERASE];
		kill_char = s.c_cc[VKILL];
#ifdef VWERASE
		werase_char = s.c_cc[VWERASE];
#else
		werase_char = CONTROL('W');
#endif

		/*
		 * Set the modes to the way we want them.
		 */
		s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
		s.c_oflag |=  (OPOST|ONLCR|TAB3);
		s.c_oflag &= ~(OCRNL|ONOCR|ONLRET);
		s.c_cc[VMIN] = 1;
		s.c_cc[VTIME] = 0;
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	ioctl(tty, TCSETAW, &s);
    }
#elif defined(TIOCGETP)
    {
	struct sgttyb s;
	static struct sgttyb save_term;
	static int saved_term = 0;

	if (on)
	{
		/*
		 * Get terminal modes.
		 */
		ioctl(tty, TIOCGETP, &s);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		if (!saved_term)
		{
			save_term = s;
			saved_term = 1;
		}
#if HAVE_OSPEED
		ospeed = s.sg_ospeed;
#endif
		erase_char = s.sg_erase;
		kill_char = s.sg_kill;
		werase_char = CONTROL('W');

		/*
		 * Set the modes to the way we want them.
		 */
		s.sg_flags |= CBREAK;
		s.sg_flags &= ~(ECHO|XTABS);
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	ioctl(tty, TIOCSETN, &s);
    }
#elif defined(_OSK)
    {
	struct sgbuf s;
	static struct sgbuf save_term;
	static int saved_term = 0;

	if (on)
	{
		/*
		 * Get terminal modes.
		 */
		_gs_opt(tty, &s);

		/*
		 * Save modes and set certain variables dependent on modes.
		 */
		if (!saved_term)
		{
			save_term = s;
			saved_term = 1;
		}
		erase_char = s.sg_bspch;
		kill_char = s.sg_dlnch;
		werase_char = CONTROL('W');

		/*
		 * Set the modes to the way we want them.
		 */
		s.sg_echo = 0;
		s.sg_eofch = 0;
		s.sg_pause = 0;
		s.sg_psch = 0;
	} else
	{
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	_ss_opt(tty, &s);
    }
#else
	/* MS-DOS, Windows, or OS2 */
#if OS2
	/* OS2 */
	LSIGNAL(SIGINT, SIG_IGN);
#endif
	erase_char = '\b';
#if DOS_DJGPPC
	kill_char = CONTROL('U');
	/*
	 * So that when we shell out or run another program, its
	 * stdin is in cooked mode.  We do not switch stdin to binary 
	 * mode if fd0 is zero, since that means we were called before
	 * tty was reopened in open_getchr, in which case we would be
	 * changing the original stdin device outside less.
	 */
	if (fd0 != 0)
		setmode(0, on ? O_BINARY : O_TEXT);
#else
	kill_char = ESC;
#endif
	werase_char = CONTROL('W');
#endif
	curr_on = on;
}

#if UNIX
/*
 * Some glue to prevent calling termcap functions if tgetent() failed.
 */
static int hardcopy;

static char * ltget_env(capname)
	char *capname;
{
	char name[64];

	if (termcap_debug)
	{
		struct env { struct env *next; char *name; char *value; };
		static struct env *envs = NULL;
		struct env *p;
		for (p = envs;  p != NULL;  p = p->next)
			if (strcmp(p->name, capname) == 0)
				return p->value;
		p = (struct env *) ecalloc(1, sizeof(struct env));
		p->name = save(capname);
		p->value = (char *) ecalloc(strlen(capname)+3, sizeof(char));
		sprintf(p->value, "<%s>", capname);
		p->next = envs;
		envs = p;
		return p->value;
	}
	snprintf(name, sizeof(name), "LESS_TERMCAP_%s", capname);
	return (lgetenv(name));
}

static int ltgetflag(capname)
	char *capname;
{
	char *s;

	if ((s = ltget_env(capname)) != NULL)
		return (*s != '\0' && *s != '0');
	if (hardcopy)
		return (0);
	return (tgetflag(capname));
}

static int ltgetnum(capname)
	char *capname;
{
	char *s;

	if ((s = ltget_env(capname)) != NULL)
		return (atoi(s));
	if (hardcopy)
		return (-1);
	return (tgetnum(capname));
}

static char * ltgetstr(capname, pp)
	char *capname;
	char **pp;
{
	char *s;

	if ((s = ltget_env(capname)) != NULL)
		return (s);
	if (hardcopy)
		return (NULL);
	return (tgetstr(capname, pp));
}
#endif /* UNIX */

/*
 * Get size of the output screen.
 */
void scrsize()
{
	char *s;
	int sys_height;
	int sys_width;
#if UNIX
	int n;
#endif

#define	DEF_SC_WIDTH	80
#if !UNIX
#define	DEF_SC_HEIGHT	25
#else
#define	DEF_SC_HEIGHT	24
#endif


	sys_width = sys_height = 0;

#if defined(WIN32)
	{
		CONSOLE_SCREEN_BUFFER_INFO scr;
		GetConsoleScreenBufferInfo(con_out, &scr);
		sys_height = scr.srWindow.Bottom - scr.srWindow.Top + 1;
		sys_width = scr.srWindow.Right - scr.srWindow.Left + 1;
	}
#elif DOS_MSC
	{
		struct videoconfig w;
		_getvideoconfig(&w);
		sys_height = w.numtextrows;
		sys_width = w.numtextcols;
	}
#elif DOS_PLATFORM
	{
		struct text_info w;
		gettextinfo(&w);
		sys_height = w.screenheight;
		sys_width = w.screenwidth;
	}
#elif OS2
	{
		int s[2];
		_scrsize(s);
		sys_width = s[0];
		sys_height = s[1];
		/*
		 * When using terminal emulators for XFree86/OS2, the
		 * _scrsize function does not work well.
		 * Call the scrsize.exe program to get the window size.
		 */
		windowid = getenv("WINDOWID");
		if (windowid != NULL)
		{
			FILE *fd = popen("scrsize", "rt");
			if (fd != NULL)
			{
				int w, h;
				fscanf(fd, "%i %i", &w, &h);
				if (w > 0 && h > 0)
				{
					sys_width = w;
					sys_height = h;
				}
				pclose(fd);
			}
		}
	}
#elif defined(TIOCGWINSZ)
	{
		struct winsize w;
		if (ioctl(2, TIOCGWINSZ, &w) == 0)
		{
			if (w.ws_row > 0)
				sys_height = w.ws_row;
			if (w.ws_col > 0)
				sys_width = w.ws_col;
		}
	}
#elif defined(WIOCGETD)
	{
		struct uwdata w;
		if (ioctl(2, WIOCGETD, &w) == 0)
		{
			if (w.uw_height > 0)
				sys_height = w.uw_height / w.uw_vs;
			if (w.uw_width > 0)
				sys_width = w.uw_width / w.uw_hs;
		}
	}
#endif

	if (sys_height > 0)
		sc_height = sys_height;
	else if ((s = lgetenv("LINES")) != NULL)
		sc_height = atoi(s);
#if UNIX
	else if ((n = ltgetnum("li")) > 0)
 		sc_height = n;
#endif
	if (sc_height <= 0)
		sc_height = DEF_SC_HEIGHT;

	if (sys_width > 0)
		sc_width = sys_width;
	else if ((s = lgetenv("COLUMNS")) != NULL)
		sc_width = atoi(s);
#if UNIX
	else if ((n = ltgetnum("co")) > 0)
 		sc_width = n;
#endif
	if (sc_width <= 0)
		sc_width = DEF_SC_WIDTH;
}

#if DOS_MSC
/*
 * Figure out how many empty loops it takes to delay a millisecond.
 */
static void get_clock()
{
	clock_t start;
	
	/*
	 * Get synchronized at the start of a tick.
	 */
	start = clock();
	while (clock() == start)
		;
	/*
	 * Now count loops till the next tick.
	 */
	start = clock();
	msec_loops = 0;
	while (clock() == start)
		msec_loops++;
	/*
	 * Convert from (loops per clock) to (loops per millisecond).
	 */
	msec_loops *= CLOCKS_PER_SEC;
	msec_loops /= 1000;
}

/*
 * Delay for a specified number of milliseconds.
 */
static void delay(int msec)
{
	long i;
	
	while (msec-- > 0)
	{
		for (i = 0;  i < msec_loops;  i++)
			(void) clock();
	}
}
#endif

/*
 * Return the characters actually input by a "special" key.
 */
char * special_key_str(int key)
{
	static char tbuf[40];
	char *s;
#if !UNIX || OS2
	static char k_right[]		= { '\340', PCK_RIGHT, 0 };
	static char k_left[]		= { '\340', PCK_LEFT, 0  };
	static char k_ctl_right[]	= { '\340', PCK_CTL_RIGHT, 0  };
	static char k_ctl_left[]	= { '\340', PCK_CTL_LEFT, 0  };
	static char k_insert[]		= { '\340', PCK_INSERT, 0  };
	static char k_delete[]		= { '\340', PCK_DELETE, 0  };
	static char k_ctl_delete[]	= { '\340', PCK_CTL_DELETE, 0  };
	static char k_ctl_backspace[]	= { '\177', 0 };
	static char k_home[]		= { '\340', PCK_HOME, 0 };
	static char k_end[]		= { '\340', PCK_END, 0 };
	static char k_up[]		= { '\340', PCK_UP, 0 };
	static char k_down[]		= { '\340', PCK_DOWN, 0 };
	static char k_backtab[]		= { '\340', PCK_SHIFT_TAB, 0 };
	static char k_pagedown[]	= { '\340', PCK_PAGEDOWN, 0 };
	static char k_pageup[]		= { '\340', PCK_PAGEUP, 0 };
	static char k_f1[]		= { '\340', PCK_F1, 0 };
#elif UNIX
	char *sp = tbuf;
#endif

	switch (key)
	{
#if OS2
	/*
	 * If windowid is not NULL, assume less is executed in 
	 * the XFree86 environment.
	 */
	case SK_RIGHT_ARROW:
		s = windowid ? ltgetstr("kr", &sp) : k_right;
		break;
	case SK_LEFT_ARROW:
		s = windowid ? ltgetstr("kl", &sp) : k_left;
		break;
	case SK_UP_ARROW:
		s = windowid ? ltgetstr("ku", &sp) : k_up;
		break;
	case SK_DOWN_ARROW:
		s = windowid ? ltgetstr("kd", &sp) : k_down;
		break;
	case SK_PAGE_UP:
		s = windowid ? ltgetstr("kP", &sp) : k_pageup;
		break;
	case SK_PAGE_DOWN:
		s = windowid ? ltgetstr("kN", &sp) : k_pagedown;
		break;
	case SK_HOME:
		s = windowid ? ltgetstr("kh", &sp) : k_home;
		break;
	case SK_END:
		s = windowid ? ltgetstr("@7", &sp) : k_end;
		break;
	case SK_DELETE:
		s = windowid ? ltgetstr("kD", &sp) : k_delete;
		if (s == NULL)
		{
			tbuf[0] = '\177';
			tbuf[1] = '\0';
			s = tbuf;
		}
		break;
#endif
#if !UNIX
	case SK_RIGHT_ARROW:
		s = k_right;
		break;
	case SK_LEFT_ARROW:
		s = k_left;
		break;
	case SK_UP_ARROW:
		s = k_up;
		break;
	case SK_DOWN_ARROW:
		s = k_down;
		break;
	case SK_PAGE_UP:
		s = k_pageup;
		break;
	case SK_PAGE_DOWN:
		s = k_pagedown;
		break;
	case SK_HOME:
		s = k_home;
		break;
	case SK_END:
		s = k_end;
		break;
	case SK_DELETE:
		s = k_delete;
		break;
#endif
#if !UNIX || OS2
	case SK_INSERT:
		s = k_insert;
		break;
	case SK_CTL_LEFT_ARROW:
		s = k_ctl_left;
		break;
	case SK_CTL_RIGHT_ARROW:
		s = k_ctl_right;
		break;
	case SK_CTL_BACKSPACE:
		s = k_ctl_backspace;
		break;
	case SK_CTL_DELETE:
		s = k_ctl_delete;
		break;
	case SK_F1:
		s = k_f1;
		break;
	case SK_BACKTAB:
		s = k_backtab;
		break;
#else
	case SK_RIGHT_ARROW:
		s = ltgetstr("kr", &sp);
		break;
	case SK_LEFT_ARROW:
		s = ltgetstr("kl", &sp);
		break;
	case SK_UP_ARROW:
		s = ltgetstr("ku", &sp);
		break;
	case SK_DOWN_ARROW:
		s = ltgetstr("kd", &sp);
		break;
	case SK_PAGE_UP:
		s = ltgetstr("kP", &sp);
		break;
	case SK_PAGE_DOWN:
		s = ltgetstr("kN", &sp);
		break;
	case SK_HOME:
		s = ltgetstr("kh", &sp);
		break;
	case SK_END:
		s = ltgetstr("@7", &sp);
		break;
	case SK_DELETE:
		s = ltgetstr("kD", &sp);
		if (s == NULL)
		{
			tbuf[0] = '\177';
			tbuf[1] = '\0';
			s = tbuf;
		}
		break;
#endif
	case SK_CONTROL_K:
		tbuf[0] = CONTROL('K');
		tbuf[1] = '\0';
		s = tbuf;
		break;
	default:
		return (NULL);
	}
	return (s);
}

/*
 * Get terminal capabilities via termcap.
 */
void get_term()
{
	termcap_debug = !isnullenv(lgetenv("LESS_TERMCAP_DEBUG"));
#if !UNIX
	auto_wrap = 1;
	ignaw = 0;
	can_goto_line = 1;
	clear_bg = 1;
	/*
	 * Set up default colors.
	 * The xx_s_width and xx_e_width vars are already initialized to 0.
	 */
#if defined(WIN32)
{
	CONSOLE_SCREEN_BUFFER_INFO scr;

	con_out_save = con_out = GetStdHandle(STD_OUTPUT_HANDLE);
	/*
	 * Always open stdin in binary. Note this *must* be done
	 * before any file operations have been done on fd0.
	 */
	SET_BINARY(0);
	GetConsoleScreenBufferInfo(con_out, &scr);
	curr_attr = scr.wAttributes;
	sy_bg_color = (curr_attr & BG_COLORS) >> 4; /* normalize */
	sy_fg_color = curr_attr & FG_COLORS;
}
#elif DOS_MSC
{
	sy_bg_color = _getbkcolor();
	sy_fg_color = _gettextcolor();
	get_clock();
}
#elif DOS_PLATFORM
{
	struct text_info w;
	gettextinfo(&w);
	sy_bg_color = (w.attribute >> 4) & 0x0F;
	sy_fg_color = (w.attribute >> 0) & 0x0F;
}
#endif
	nm_fg_color = sy_fg_color;
	nm_bg_color = sy_bg_color;
	bo_fg_color = 11;
	bo_bg_color = 0;
	ul_fg_color = 9;
	ul_bg_color = 0;
	so_fg_color = 15;
	so_bg_color = 9;
	bl_fg_color = 15;
	bl_bg_color = 0;
	sgr_mode = 0;

	/*
	 * Get size of the screen.
	 */
	scrsize();
	pos_init();


#else /* UNIX */
{
	char *sp;
	char *t1, *t2;
	char *term;
	/*
	 * Some termcap libraries assume termbuf is static
	 * (accessible after tgetent returns).
	 */
	static char termbuf[TERMBUF_SIZE];
	static char sbuf[TERMSBUF_SIZE];

#if OS2
	/*
	 * Make sure the termcap database is available.
	 */
	sp = lgetenv("TERMCAP");
	if (isnullenv(sp))
	{
		char *termcap;
		if ((sp = homefile("termcap.dat")) != NULL)
		{
			termcap = (char *) ecalloc(strlen(sp)+9, sizeof(char));
			sprintf(termcap, "TERMCAP=%s", sp);
			free(sp);
			putenv(termcap);
		}
	}
#endif
	/*
	 * Find out what kind of terminal this is.
	 */
	if ((term = lgetenv("TERM")) == NULL)
		term = DEFAULT_TERM;
	hardcopy = 0;
	/* {{ Should probably just pass NULL instead of termbuf. }} */
	if (tgetent(termbuf, term) != TGETENT_OK)
		hardcopy = 1;
	if (ltgetflag("hc"))
		hardcopy = 1;

	/*
	 * Get size of the screen.
	 */
	scrsize();
	pos_init();

	auto_wrap = ltgetflag("am");
	ignaw = ltgetflag("xn");
	above_mem = ltgetflag("da");
	below_mem = ltgetflag("db");
	clear_bg = ltgetflag("ut");

	/*
	 * Assumes termcap variable "sg" is the printing width of:
	 * the standout sequence, the end standout sequence,
	 * the underline sequence, the end underline sequence,
	 * the boldface sequence, and the end boldface sequence.
	 */
	if ((so_s_width = ltgetnum("sg")) < 0)
		so_s_width = 0;
	so_e_width = so_s_width;

	bo_s_width = bo_e_width = so_s_width;
	ul_s_width = ul_e_width = so_s_width;
	bl_s_width = bl_e_width = so_s_width;

#if HILITE_SEARCH
	if (so_s_width > 0 || so_e_width > 0)
		/*
		 * Disable highlighting by default on magic cookie terminals.
		 * Turning on highlighting might change the displayed width
		 * of a line, causing the display to get messed up.
		 * The user can turn it back on with -g, 
		 * but she won't like the results.
		 */
		hilite_search = 0;
#endif

	/*
	 * Get various string-valued capabilities.
	 */
	sp = sbuf;

#if HAVE_OSPEED
	sc_pad = ltgetstr("pc", &sp);
	if (sc_pad != NULL)
		PC = *sc_pad;
#endif

	sc_s_keypad = ltgetstr("ks", &sp);
	if (sc_s_keypad == NULL)
		sc_s_keypad = "";
	sc_e_keypad = ltgetstr("ke", &sp);
	if (sc_e_keypad == NULL)
		sc_e_keypad = "";
	kent = ltgetstr("@8", &sp);

	sc_s_mousecap = ltgetstr("MOUSE_START", &sp);
	if (sc_s_mousecap == NULL)
		sc_s_mousecap = ESCS "[?1000h" ESCS "[?1006h";
	sc_e_mousecap = ltgetstr("MOUSE_END", &sp);
	if (sc_e_mousecap == NULL)
		sc_e_mousecap = ESCS "[?1006l" ESCS "[?1000l";

	sc_init = ltgetstr("ti", &sp);
	if (sc_init == NULL)
		sc_init = "";

	sc_deinit= ltgetstr("te", &sp);
	if (sc_deinit == NULL)
		sc_deinit = "";

	sc_eol_clear = ltgetstr("ce", &sp);
	if (sc_eol_clear == NULL || *sc_eol_clear == '\0')
	{
		missing_cap = 1;
		sc_eol_clear = "";
	}

	sc_eos_clear = ltgetstr("cd", &sp);
	if (below_mem && (sc_eos_clear == NULL || *sc_eos_clear == '\0'))
	{
		missing_cap = 1;
		sc_eos_clear = "";
	}

	sc_clear = ltgetstr("cl", &sp);
	if (sc_clear == NULL || *sc_clear == '\0')
	{
		missing_cap = 1;
		sc_clear = "\n\n";
	}

	sc_move = ltgetstr("cm", &sp);
	if (sc_move == NULL || *sc_move == '\0')
	{
		/*
		 * This is not an error here, because we don't 
		 * always need sc_move.
		 * We need it only if we don't have home or lower-left.
		 */
		sc_move = "";
		can_goto_line = 0;
	} else
		can_goto_line = 1;

	tmodes("so", "se", &sc_s_in, &sc_s_out, "", "", &sp);
	tmodes("us", "ue", &sc_u_in, &sc_u_out, sc_s_in, sc_s_out, &sp);
	tmodes("md", "me", &sc_b_in, &sc_b_out, sc_s_in, sc_s_out, &sp);
	tmodes("mb", "me", &sc_bl_in, &sc_bl_out, sc_s_in, sc_s_out, &sp);

	sc_visual_bell = ltgetstr("vb", &sp);
	if (sc_visual_bell == NULL)
		sc_visual_bell = "";

	if (ltgetflag("bs"))
		sc_backspace = "\b";
	else
	{
		sc_backspace = ltgetstr("bc", &sp);
		if (sc_backspace == NULL || *sc_backspace == '\0')
			sc_backspace = "\b";
	}

	/*
	 * Choose between using "ho" and "cm" ("home" and "cursor move")
	 * to move the cursor to the upper left corner of the screen.
	 */
	t1 = ltgetstr("ho", &sp);
	if (t1 == NULL)
		t1 = "";
	if (*sc_move == '\0')
		t2 = "";
	else
	{
		strcpy(sp, tgoto(sc_move, 0, 0));
		t2 = sp;
		sp += strlen(sp) + 1;
	}
	sc_home = cheaper(t1, t2, "|\b^");

	/*
	 * Choose between using "ll" and "cm"  ("lower left" and "cursor move")
	 * to move the cursor to the lower left corner of the screen.
	 */
	t1 = ltgetstr("ll", &sp);
	if (t1 == NULL)
		t1 = "";
	if (*sc_move == '\0')
		t2 = "";
	else
	{
		strcpy(sp, tgoto(sc_move, 0, sc_height-1));
		t2 = sp;
		sp += strlen(sp) + 1;
	}
	sc_lower_left = cheaper(t1, t2, "\r");

	/*
	 * Get carriage return string.
	 */
	sc_return = ltgetstr("cr", &sp);
	if (sc_return == NULL)
		sc_return = "\r";

	/*
	 * Choose between using "al" or "sr" ("add line" or "scroll reverse")
	 * to add a line at the top of the screen.
	 */
	t1 = ltgetstr("al", &sp);
	if (t1 == NULL)
		t1 = "";
	t2 = ltgetstr("sr", &sp);
	if (t2 == NULL)
		t2 = "";
#if OS2
	if (*t1 == '\0' && *t2 == '\0')
		sc_addline = "";
	else
#endif
	if (above_mem)
		sc_addline = t1;
	else
		sc_addline = cheaper(t1, t2, "");
	if (*sc_addline == '\0')
	{
		/*
		 * Force repaint on any backward movement.
		 */
		no_back_scroll = 1;
	}
}
#endif
}

#if UNIX
/*
 * Return the cost of displaying a termcap string.
 * We use the trick of calling tputs, but as a char printing function
 * we give it inc_costcount, which just increments "costcount".
 * This tells us how many chars would be printed by using this string.
 * {{ Couldn't we just use strlen? }}
 */
static int costcount;

/*ARGSUSED*/
static int inc_costcount(c)
	int c;
{
	costcount++;
	return (c);
}

static int cost(t)
	char *t;
{
	costcount = 0;
	tputs(t, sc_height, inc_costcount);
	return (costcount);
}

/*
 * Return the "best" of the two given termcap strings.
 * The best, if both exist, is the one with the lower 
 * cost (see cost() function).
 */
static char * cheaper(t1, t2, def)
	char *t1, *t2;
	char *def;
{
	if (*t1 == '\0' && *t2 == '\0')
	{
		missing_cap = 1;
		return (def);
	}
	if (*t1 == '\0')
		return (t2);
	if (*t2 == '\0')
		return (t1);
	if (cost(t1) < cost(t2))
		return (t1);
	return (t2);
}

static void tmodes(incap, outcap, instr, outstr, def_instr, def_outstr, spp)
	char *incap;
	char *outcap;
	char **instr;
	char **outstr;
	char *def_instr;
	char *def_outstr;
	char **spp;
{
	*instr = ltgetstr(incap, spp);
	if (*instr == NULL)
	{
		/* Use defaults. */
		*instr = def_instr;
		*outstr = def_outstr;
		return;
	}

	*outstr = ltgetstr(outcap, spp);
	if (*outstr == NULL)
		/* No specific out capability; use "me". */
		*outstr = ltgetstr("me", spp);
	if (*outstr == NULL)
		/* Don't even have "me"; use a null string. */
		*outstr = "";
}

#endif


/*
 * Below are the functions which perform all the 
 * terminal-specific screen manipulation.
 */


#if !UNIX

#if defined(WIN32)
static void _settextposition(int row, int col)
{
	COORD cpos;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(con_out, &csbi);
	cpos.X = csbi.srWindow.Left + (col - 1);
	cpos.Y = csbi.srWindow.Top + (row - 1);
	SetConsoleCursorPosition(con_out, cpos);
}
#endif

/*
 * Initialize the screen to the correct color at startup.
 */
static void initcolor()
{
#if DOS_DJGPPC || DOS_BORLAND
	intensevideo();
#endif
	SETCOLORS(nm_fg_color, nm_bg_color);
}
#endif

#if defined(WIN32)

/*
 * Termcap-like init with a private win32 console.
 */
static void win32_init_term()
{
	CONSOLE_SCREEN_BUFFER_INFO scr;
	COORD size;

	if (con_out_save == INVALID_HANDLE_VALUE)
		return;

	GetConsoleScreenBufferInfo(con_out_save, &scr);

	if (con_out_ours == INVALID_HANDLE_VALUE)
	{
		DWORD output_mode;

		/*
		 * Create our own screen buffer, so that we
		 * may restore the original when done.
		 */
		con_out_ours = CreateConsoleScreenBuffer(
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			(LPSECURITY_ATTRIBUTES) NULL,
			CONSOLE_TEXTMODE_BUFFER,
			(LPVOID) NULL);
		/*
		 * Enable virtual terminal processing, if available.
		 */
		GetConsoleMode(con_out_ours, &output_mode);
		vt_enabled = SetConsoleMode(con_out_ours,
			       output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	size.X = scr.srWindow.Right - scr.srWindow.Left + 1;
	size.Y = scr.srWindow.Bottom - scr.srWindow.Top + 1;
	SetConsoleScreenBufferSize(con_out_ours, size);
	SetConsoleActiveScreenBuffer(con_out_ours);
	con_out = con_out_ours;
}

/*
 * Restore the startup console.
 */
static void win32_deinit_term()
{
	if (con_out_save == INVALID_HANDLE_VALUE)
		return;
	if (quitting)
		(void) CloseHandle(con_out_ours);
	SetConsoleActiveScreenBuffer(con_out_save);
	con_out = con_out_save;
}

#endif

/*
 * Configure the termimal so mouse clicks and wheel moves 
 * produce input to less.
 */
void init_mouse()
{
	if (!mousecap)
		return;
#if UNIX
	tputs(sc_s_mousecap, sc_height, putchr);
#elif defined WIN32
	SetConsoleMode(tty, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT
			    | ENABLE_EXTENDED_FLAGS /* disable quick edit */);
#endif
}

/*
 * Configure the terminal so mouse clicks and wheel moves
 * are handled by the system (so text can be selected, etc).
 */
void deinit_mouse()
{
	if (!mousecap)
		return;
#if UNIX
	tputs(sc_e_mousecap, sc_height, putchr);
#elif defined WIN32
	SetConsoleMode(tty, ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS
			    | (console_mode & ENABLE_QUICK_EDIT_MODE));
#endif
}

/*
 * Initialize terminal
 */
void init()
{
#if UNIX
	if (!(quit_if_one_screen && one_screen))
	{
		if (!no_init)
			tputs(sc_init, sc_height, putchr);
		if (!no_keypad)
			tputs(sc_s_keypad, sc_height, putchr);
		init_mouse();
	}
	if (top_scroll) 
	{
		int i;

		/*
		 * This is nice to terminals with no alternate screen,
		 * but with saved scrolled-off-the-top lines.  This way,
		 * no previous line is lost, but we start with a whole
		 * screen to ourself.
		 */
		for (i = 1; i < sc_height; i++)
			putchr('\n');
	} else
		line_left();
#else
#if defined(WIN32)
	GetConsoleMode(con_out, &init_output_mode);
	if (!(quit_if_one_screen && one_screen))
	{
		if (!no_init)
			win32_init_term();

		init_mouse();
	}
	else {
		/*
		 * Enable virtual terminal processing, if available.
		 */
		vt_enabled = SetConsoleMode(con_out, init_output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
	initcolor();
	flush();
#endif
	init_done = 1;
}

/*
 * Deinitialize terminal
 */
void deinit()
{
	if (!init_done)
		return;
#if UNIX
	if (!(quit_if_one_screen && one_screen))
	{
		deinit_mouse();
		if (!no_keypad)
			tputs(sc_e_keypad, sc_height, putchr);
		if (!no_init)
			tputs(sc_deinit, sc_height, putchr);
	}
#else
	/* Restore system colors. */
	SETCOLORS(sy_fg_color, sy_bg_color);
#if defined(WIN32)
	if (!(quit_if_one_screen && one_screen))
	{
		deinit_mouse();
		SetConsoleMode(con_out, init_output_mode);
		if (!no_init) {
			win32_deinit_term();

		}
	}
#else
	/* Need clreol to make SETCOLORS take effect. */
	clreol();
#endif
#endif
	init_done = 0;
}

/*
 * Home cursor (move to upper left corner of screen).
 */
void home()
{
#if UNIX
	tputs(sc_home, 1, putchr);
#else
	flush();
	_settextposition(1,1);
#endif
}

/*
 * Add a blank line (called with cursor at home).
 * Should scroll the display down.
 */
void add_line()
{
#if UNIX
	tputs(sc_addline, sc_height, putchr);
#else
	flush();

#if defined(WIN32)
    {
	CHAR_INFO fillchar;
	SMALL_RECT rcSrc, rcClip;
	COORD new_org;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(con_out,&csbi);

	/* The clip rectangle is the entire visible screen. */
	rcClip.Left = csbi.srWindow.Left;
	rcClip.Top = csbi.srWindow.Top;
	rcClip.Right = csbi.srWindow.Right;
	rcClip.Bottom = csbi.srWindow.Bottom;

	/* The source rectangle is the visible screen minus the last line. */
	rcSrc = rcClip;
	rcSrc.Bottom--;

	/* Move the top left corner of the source window down one row. */
	new_org.X = rcSrc.Left;
	new_org.Y = rcSrc.Top + 1;

	/* Fill the right character and attributes. */
	fillchar.Char.AsciiChar = ' ';
	curr_attr = MAKEATTR(nm_fg_color, nm_bg_color);
	fillchar.Attributes = curr_attr;
	ScrollConsoleScreenBuffer(con_out, &rcSrc, &rcClip, new_org, &fillchar);
	_settextposition(1,1);
    }
#elif DOS_MSC
	_scrolltextwindow(_GSCROLLDOWN);
	_settextposition(1,1);
#elif DOS_PLATFORM
	movetext(1,1, sc_width,sc_height-1, 1,2);
	gotoxy(1,1);
	clreol();
#endif
#endif
}


#if defined(WIN32)
/*
 * Clear the screen.
 */
static void win32_clear()
{
	/*
	 * This will clear only the currently visible rows of the NT
	 * console buffer, which means none of the precious scrollback
	 * rows are touched making for faster scrolling.  Note that, if
	 * the window has fewer columns than the console buffer (i.e.
	 * there is a horizontal scrollbar as well), the entire width
	 * of the visible rows will be cleared.
	 */
	COORD topleft;
	DWORD nchars;
	DWORD winsz;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	/* get the number of cells in the current buffer */
	GetConsoleScreenBufferInfo(con_out, &csbi);
	winsz = csbi.dwSize.X * (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
	topleft.X = 0;
	topleft.Y = csbi.srWindow.Top;

	curr_attr = MAKEATTR(nm_fg_color, nm_bg_color);
	FillConsoleOutputCharacter(con_out, ' ', winsz, topleft, &nchars);
	FillConsoleOutputAttribute(con_out, curr_attr, winsz, topleft, &nchars);
}

/*
 * Remove the n topmost lines and scroll everything below it in the 
 * window upward.
 */
void win32_scroll_up(n)
	int n;
{
	SMALL_RECT rcSrc, rcClip;
	CHAR_INFO fillchar;
	COORD topleft;
	COORD new_org;
	DWORD nchars;
	DWORD size;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (n <= 0)
		return;

	if (n >= sc_height - 1)
	{
		win32_clear();
		_settextposition(1,1);
		return;
	}

	/* Get the extent of what will remain visible after scrolling. */
	GetConsoleScreenBufferInfo(con_out, &csbi);
	rcSrc.Left    = csbi.srWindow.Left;
	rcSrc.Top     = csbi.srWindow.Top + n;
	rcSrc.Right   = csbi.srWindow.Right;
	rcSrc.Bottom  = csbi.srWindow.Bottom;

	/* Get the clip rectangle. */
	rcClip.Left   = rcSrc.Left;
	rcClip.Top    = csbi.srWindow.Top;
	rcClip.Right  = rcSrc.Right;
	rcClip.Bottom = rcSrc.Bottom ;

	/* Move the source text to the top of the screen. */
	new_org.X = rcSrc.Left;
	new_org.Y = rcClip.Top;

	/* Fill the right character and attributes. */
	fillchar.Char.AsciiChar = ' ';
	fillchar.Attributes = MAKEATTR(nm_fg_color, nm_bg_color);

	/* Scroll the window. */
	SetConsoleTextAttribute(con_out, fillchar.Attributes);
	ScrollConsoleScreenBuffer(con_out, &rcSrc, &rcClip, new_org, &fillchar);

	/* Clear remaining lines at bottom. */
	topleft.X = csbi.dwCursorPosition.X;
	topleft.Y = rcSrc.Bottom - n;
	size = (n * csbi.dwSize.X) + (rcSrc.Right - topleft.X);
	FillConsoleOutputCharacter(con_out, ' ', size, topleft,
		&nchars);
	FillConsoleOutputAttribute(con_out, fillchar.Attributes, size, topleft,
		&nchars);
	SetConsoleTextAttribute(con_out, curr_attr);

	/* Move cursor n lines up from where it was. */
	csbi.dwCursorPosition.Y -= n;
	SetConsoleCursorPosition(con_out, csbi.dwCursorPosition);
}
#endif

/*
 * Move cursor to lower left corner of screen.
 */
void lower_left()
{
	if (!init_done)
		return;
#if UNIX
	tputs(sc_lower_left, 1, putchr);
#else
	flush();
	_settextposition(sc_height, 1);
#endif
}

/*
 * Move cursor to left position of current line.
 */
void line_left()
{
#if UNIX
	tputs(sc_return, 1, putchr);
#else
	int row;
	flush();
#if defined(WIN32)
	{
		CONSOLE_SCREEN_BUFFER_INFO scr;
		GetConsoleScreenBufferInfo(con_out, &scr);
		row = scr.dwCursorPosition.Y - scr.srWindow.Top + 1;
	}
#else
#if DOS_BORLAND || DOS_DJGPPC
		row = wherey();
#else
	{
		struct rccoord tpos = _gettextposition();
		row = tpos.row;
	}
#endif
#endif
	_settextposition(row, 1);
#endif
}

/*
 * Check if the console size has changed and reset internals 
 * (in lieu of SIGWINCH for WIN32).
 */
void check_winch()
{
#if defined(WIN32)
	CONSOLE_SCREEN_BUFFER_INFO scr;
	COORD size;

	if (con_out == INVALID_HANDLE_VALUE)
		return;
 
	flush();
	GetConsoleScreenBufferInfo(con_out, &scr);
	size.Y = scr.srWindow.Bottom - scr.srWindow.Top + 1;
	size.X = scr.srWindow.Right - scr.srWindow.Left + 1;
	if (size.Y != sc_height || size.X != sc_width)
	{
		sc_height = size.Y;
		sc_width = size.X;
		if (!no_init && con_out_ours == con_out)
			SetConsoleScreenBufferSize(con_out, size);
		pos_init();
		wscroll = (sc_height + 1) / 2;
		screen_trashed = 1;
	}
#endif
}

/*
 * Goto a specific line on the screen.
 */
void goto_line(sindex)
	int sindex;
{
#if UNIX
	tputs(tgoto(sc_move, 0, sindex), 1, putchr);
#else
	flush();
	_settextposition(sindex+1, 1);
#endif
}

#if DOS_PLATFORM && !DOS_DJGPPC
/*
 * Create an alternate screen which is all white.
 * This screen is used to create a "flash" effect, by displaying it
 * briefly and then switching back to the normal screen.
 * {{ Yuck!  There must be a better way to get a visual bell. }}
 */
static void create_flash() {
#if DOS_MSC
	struct videoconfig w;
	char *blanks;
	int row, col;
	
	_getvideoconfig(&w);
	videopages = w.numvideopages;
	if (videopages < 2)
	{
		at_enter(AT_STANDOUT);
		at_exit();
	} else
	{
		_setactivepage(1);
		at_enter(AT_STANDOUT);
		blanks = (char *) ecalloc(w.numtextcols, sizeof(char));
		for (col = 0;  col < w.numtextcols;  col++)
			blanks[col] = ' ';
		for (row = w.numtextrows;  row > 0;  row--)
			_outmem(blanks, w.numtextcols);
		_setactivepage(0);
		_setvisualpage(0);
		free(blanks);
		at_exit();
	}
#else	/*BORLANDC*/
	int n;

	whitescreen = (unsigned short *) malloc(sc_width * sc_height * sizeof(short));
	if (whitescreen == NULL)
		return;
	for (n = 0;  n < sc_width * sc_height;  n++)
		whitescreen[n] = 0x7020;
#endif
	flash_created = 1;
}
#endif

/*
 * Output the "visual bell", if there is one.
 */
void vbell()
{
#if UNIX
	if (*sc_visual_bell == '\0')
		return;
	tputs(sc_visual_bell, sc_height, putchr);
#elif defined WIN32
	/* paint screen with an inverse color */
	clear();

	/* leave it displayed for 100 msec. */
	Sleep(100);

	/* restore with a redraw */
	repaint();
#elif DOS_MSC
	/*
	 * Create a flash screen on the second video page.
	 * Switch to that page, then switch back.
	 */
	if (!flash_created)
		create_flash();
	if (videopages < 2)
		return;
	_setvisualpage(1);
	delay(100);
	_setvisualpage(0);
#elif DOS_DJGPPC
	ScreenVisualBell();
#elif DOS_BORLAND
	unsigned short *currscreen;

	/*
	 * Get a copy of the current screen.
	 * Display the flash screen.
	 * Then restore the old screen.
	 */
	if (!flash_created)
		create_flash();
	if (whitescreen == NULL)
		return;
	currscreen = (unsigned short *) 
		malloc(sc_width * sc_height * sizeof(short));
	if (currscreen == NULL) return;
	gettext(1, 1, sc_width, sc_height, currscreen);
	puttext(1, 1, sc_width, sc_height, whitescreen);
	delay(100);
	puttext(1, 1, sc_width, sc_height, currscreen);
	free(currscreen);
#endif
}

/*
 * Make a noise.
 */
static void beep()
{
#if UNIX
	putchr(CONTROL('G'));
#elif defined WIN32
	MessageBeep(0);
#else
	write(1, "\7", 1);
#endif
}

/*
 * Ring the terminal bell.
 */
void bell()
{
	if (quiet == VERY_QUIET)
		vbell();
	else
		beep();
}

/*
 * Clear the screen.
 */
void clear()
{
#if UNIX
	tputs(sc_clear, sc_height, putchr);
#else
	flush();
#if defined(WIN32)
	win32_clear();
#else
	_clearscreen(_GCLEARSCREEN);
#endif
#endif
}

/*
 * Clear from the cursor to the end of the cursor's line.
 * {{ This must not move the cursor. }}
 */
void clear_eol()
{
#if UNIX
	tputs(sc_eol_clear, 1, putchr);
#elif defined(WIN32)
	DWORD           nchars;
	COORD           cpos;
	CONSOLE_SCREEN_BUFFER_INFO scr;

	flush();
	memset(&scr, 0, sizeof(scr));
	GetConsoleScreenBufferInfo(con_out, &scr);
	cpos.X = scr.dwCursorPosition.X;
	cpos.Y = scr.dwCursorPosition.Y;
	curr_attr = MAKEATTR(nm_fg_color, nm_bg_color);
	FillConsoleOutputAttribute(con_out, curr_attr, scr.dwSize.X - cpos.X, cpos, &nchars);
	FillConsoleOutputCharacter(con_out, ' ', scr.dwSize.X - cpos.X, cpos, &nchars);
#elif DOS_MSC
	short top, left;
	short bot, right;
	struct rccoord tpos;
	
	flush();
	/*
	 * Save current state.
	 */
	tpos = _gettextposition();
	_gettextwindow(&top, &left, &bot, &right);
	/*
	 * Set a temporary window to the current line,
	 * from the cursor's position to the right edge of the screen.
	 * Then clear that window.
	 */
	_settextwindow(tpos.row, tpos.col, tpos.row, sc_width);
	_clearscreen(_GWINDOW);
	/*
	 * Restore state.
	 */
	_settextwindow(top, left, bot, right);
	_settextposition(tpos.row, tpos.col);
#elif DOS_PLATFORM
	flush();
	clreol();
#endif
}

/*
 * Clear the current line.
 * Clear the screen if there's off-screen memory below the display.
 */
static void clear_eol_bot()
{
#if !UNIX
	clear_eol();
#else
	if (below_mem)
		tputs(sc_eos_clear, 1, putchr);
	else
		tputs(sc_eol_clear, 1, putchr);
#endif
}

/*
 * Clear the bottom line of the display.
 * Leave the cursor at the beginning of the bottom line.
 */
void clear_bot()
{
	/*
	 * If we're in a non-normal attribute mode, temporarily exit
	 * the mode while we do the clear.  Some terminals fill the
	 * cleared area with the current attribute.
	 */
	if (oldbot)
		lower_left();
	else
		line_left();

	if (attrmode == AT_NORMAL)
		clear_eol_bot();
	else
	{
		int saved_attrmode = attrmode;

		at_exit();
		clear_eol_bot();
		at_enter(saved_attrmode);
	}
}

void at_enter(int attr)
{
	attr = apply_at_specials(attr);

#if UNIX
	/* The one with the most priority is last.  */
	if (attr & AT_UNDERLINE)
		tputs(sc_u_in, 1, putchr);
	if (attr & AT_BOLD)
		tputs(sc_b_in, 1, putchr);
	if (attr & AT_BLINK)
		tputs(sc_bl_in, 1, putchr);
	if (attr & AT_STANDOUT)
		tputs(sc_s_in, 1, putchr);
#else
	flush();
	/* The one with the most priority is first.  */
	if (attr & AT_STANDOUT)
	{
		SETCOLORS(so_fg_color, so_bg_color);
	} else if (attr & AT_BLINK)
	{
		SETCOLORS(bl_fg_color, bl_bg_color);
	}
	else if (attr & AT_BOLD)
	{
		SETCOLORS(bo_fg_color, bo_bg_color);
	}
	else if (attr & AT_UNDERLINE)
	{
		SETCOLORS(ul_fg_color, ul_bg_color);
	}
#endif

	attrmode = attr;
}

void at_exit()
{
#if UNIX
	/* Undo things in the reverse order we did them.  */
	if (attrmode & AT_STANDOUT)
		tputs(sc_s_out, 1, putchr);
	if (attrmode & AT_BLINK)
		tputs(sc_bl_out, 1, putchr);
	if (attrmode & AT_BOLD)
		tputs(sc_b_out, 1, putchr);
	if (attrmode & AT_UNDERLINE)
		tputs(sc_u_out, 1, putchr);
#else
	flush();
	SETCOLORS(nm_fg_color, nm_bg_color);
#endif

	attrmode = AT_NORMAL;
}

void at_switch(int attr)
{
	int new_attrmode = apply_at_specials(attr);
	int ignore_modes = AT_ANSI;

	if ((new_attrmode & ~ignore_modes) != (attrmode & ~ignore_modes))
	{
		at_exit();
		at_enter(attr);
	}
}

int is_at_equiv(int attr1, int attr2)
{
	attr1 = apply_at_specials(attr1);
	attr2 = apply_at_specials(attr2);

	return (attr1 == attr2);
}

int apply_at_specials(int attr)
{
	if (attr & AT_BINARY)
		attr |= binattr;
	if (attr & AT_HILITE)
		attr |= AT_STANDOUT;
	attr &= ~(AT_BINARY|AT_HILITE);

	return attr;
}

/*
 * Output a plain backspace, without erasing the previous char.
 */
void putbs()
{
	if (termcap_debug)
		putstr("<bs>");
	else
	{
#if UNIX
	tputs(sc_backspace, 1, putchr);
#else
	int row, col;

	flush();
	{
#if defined(WIN32)
		CONSOLE_SCREEN_BUFFER_INFO scr;
		GetConsoleScreenBufferInfo(con_out, &scr);
		row = scr.dwCursorPosition.Y - scr.srWindow.Top + 1;
		col = scr.dwCursorPosition.X - scr.srWindow.Left + 1;
#elif DOS_MSC
		struct rccoord tpos;
		tpos = _gettextposition();
		row = tpos.row;
		col = tpos.col;
#elif DOS_PLATFORM
		row = wherey();
		col = wherex();
#endif
#endif
	}
	if (col <= 1)
		return;
	_settextposition(row, col-1);
	}
}

#if defined(WIN32)
/*
 * Determine whether an input character is waiting to be read.
 */
int win32_kbhit()
{
	INPUT_RECORD ip;
	DWORD read;

	if (keyCount > 0)
		return (true);

	currentKey.ascii = 0;
	currentKey.scan = 0;

	if (x11mouseCount > 0)
	{
		currentKey.ascii = x11mousebuf[x11mousePos++];
		--x11mouseCount;
		keyCount = 1;
		return (true);
	}

	/*
	 * Wait for a real key-down event, but
	 * ignore SHIFT and CONTROL key events.
	 */
	do
	{
		PeekConsoleInput(tty, &ip, 1, &read);
		if (read == 0)
			return (false);
		ReadConsoleInput(tty, &ip, 1, &read);
		/* generate an X11 mouse sequence from the mouse event */
		if (mousecap && ip.EventType == MOUSE_EVENT &&
		    ip.Event.MouseEvent.dwEventFlags != MOUSE_MOVED)
		{
			x11mousebuf[3] = X11MOUSE_OFFSET + ip.Event.MouseEvent.dwMousePosition.X + 1;
			x11mousebuf[4] = X11MOUSE_OFFSET + ip.Event.MouseEvent.dwMousePosition.Y + 1;
			switch (ip.Event.MouseEvent.dwEventFlags)
			{
			case 0: /* press or release */
				if (ip.Event.MouseEvent.dwButtonState == 0)
					x11mousebuf[2] = X11MOUSE_OFFSET + X11MOUSE_BUTTON_REL;
				else if (ip.Event.MouseEvent.dwButtonState & (FROM_LEFT_3RD_BUTTON_PRESSED | FROM_LEFT_4TH_BUTTON_PRESSED))
					continue;
				else
					x11mousebuf[2] = X11MOUSE_OFFSET + X11MOUSE_BUTTON1 + ((int)ip.Event.MouseEvent.dwButtonState << 1);
				break;
			case MOUSE_WHEELED:
				x11mousebuf[2] = X11MOUSE_OFFSET + (((int)ip.Event.MouseEvent.dwButtonState < 0) ? X11MOUSE_WHEEL_DOWN : X11MOUSE_WHEEL_UP);
				break;
			default:
				continue;
			}
			x11mousePos = 0;
			x11mouseCount = 5;
			currentKey.ascii = ESC;
			keyCount = 1;
			return (true);
		}
	} while (ip.EventType != KEY_EVENT ||
		ip.Event.KeyEvent.bKeyDown != true ||
		ip.Event.KeyEvent.wVirtualScanCode == 0 ||
		ip.Event.KeyEvent.wVirtualKeyCode == VK_SHIFT ||
		ip.Event.KeyEvent.wVirtualKeyCode == VK_CONTROL ||
		ip.Event.KeyEvent.wVirtualKeyCode == VK_MENU);
		
	currentKey.ascii = ip.Event.KeyEvent.uChar.AsciiChar;
	currentKey.scan = ip.Event.KeyEvent.wVirtualScanCode;
	keyCount = ip.Event.KeyEvent.wRepeatCount;

	if (ip.Event.KeyEvent.dwControlKeyState & 
		(LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
	{
		switch (currentKey.scan)
		{
		case PCK_ALT_E:     /* letter 'E' */
			currentKey.ascii = 0;
			break;
		}
	} else if (ip.Event.KeyEvent.dwControlKeyState & 
		(LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
	{
		switch (currentKey.scan)
		{
		case PCK_RIGHT: /* right arrow */
			currentKey.scan = PCK_CTL_RIGHT;
			break;
		case PCK_LEFT: /* left arrow */
			currentKey.scan = PCK_CTL_LEFT;
			break;
		case PCK_DELETE: /* delete */
			currentKey.scan = PCK_CTL_DELETE;
			break;
		}
	} else if (ip.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
	{
		switch (currentKey.scan)
		{
		case PCK_SHIFT_TAB: /* tab */
			currentKey.ascii = 0;
			break;
		}
	}

	return (true);
}

/*
 * Read a character from the keyboard.
 */
char WIN32getch()
{
	int ascii;

	if (pending_scancode)
	{
		pending_scancode = 0;
		return ((char)(currentKey.scan & 0x00FF));
	}

	do {
		while (win32_kbhit() == false)
		{
			Sleep(20);
			if (ABORT_SIGS())
				return ('\003');
			continue;
		}
		keyCount --;
		ascii = currentKey.ascii;
		/*
		 * On PC's, the extended keys return a 2 byte sequence beginning 
		 * with '00', so if the ascii code is 00, the next byte will be 
		 * the lsb of the scan code.
		 */
		pending_scancode = (ascii == 0x00);
	} while (pending_scancode &&
		(currentKey.scan == PCK_CAPS_LOCK || currentKey.scan == PCK_NUM_LOCK));

	return ((char)ascii);
}
#endif

#if !UNIX
/*
 */
void WIN32setcolors(int fg, int bg)
{
	SETCOLORS(fg, bg);
}

/*
 */
void WIN32textout(char *text, int len)
{
#if defined(WIN32)
	DWORD written;
	if (utf_mode == 2)
	{
		/*
		 * We've got UTF-8 text in a non-UTF-8 console.  Convert it to
		 * wide and use WriteConsoleW.
		 */
		WCHAR wtext[1024];
		len = MultiByteToWideChar(CP_UTF8, 0, text, len, wtext,
					  sizeof(wtext)/sizeof(*wtext));
		WriteConsoleW(con_out, wtext, len, &written, NULL);
	} else
		WriteConsoleA(con_out, text, len, &written, NULL);
#else
	char c = text[len];
	text[len] = '\0';
	cputs(text);
	text[len] = c;
#endif
}
#endif

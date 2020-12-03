/*@@copyright@@*/

/*
 * Include file for defines and constants.
 */

#ifndef SECURE
#include "defines.h"
#endif

#ifndef LESSDEF_H
#define LESSDEF_H

/* platform detection helpers. keep this @ the top */
#if defined DOS_DJGPPC || defined DOS_BORLAND || defined DOS_MSC
#   define DOS_PLATFORM 1
#endif

#if defined WIN32 || defined DOS_PLATFORM
//  !UNIX
#else
#   define UNIX 1
#endif


#define MIN_LINENUM_WIDTH  7	/* Min printing width of a line number */
#define MAX_UTF_CHAR_LEN   6	/* Max bytes in one UTF-8 char */

#define	NULL_POSITION	((POSITION)(-1))

/*
 * Upper bound on the string length of an integer converted to string.
 * 302 / 1000 is ceil (log10 (2.0)).  Subtract 1 for the sign bit;
 * add 1 for integer division truncation; add 1 more for a minus sign.
 */
#define INT_STRLEN_BOUND(t) ((sizeof(t) * CHAR_BIT - 1) * 302 / 1000 + 1 + 1)

#define	EOI		(-1)
#define	READ_INTR	(-2)

/* A fraction is represented by an int n; the fraction is n/NUM_FRAC_DENOM */
#define NUM_FRAC_DENOM			1000000
#define NUM_LOG_FRAC_DENOM		6

/* Flags */
/* How quiet should we be? */
#define	NOT_QUIET	0	/* Ring bell at eof and for errors */
#define	LITTLE_QUIET	1	/* Ring bell only for errors */
#define	VERY_QUIET	2	/* Never ring bell */

/* How should we prompt? */
#define	PR_SHORT	0	/* Prompt with colon */
#define	PR_MEDIUM	1	/* Prompt with message */
#define	PR_LONG		2	/* Prompt with longer message */

/* How should we handle backspaces? */
#define	BS_SPECIAL	0	/* Do special things for underlining and bold */
#define	BS_NORMAL	1	/* \b treated as normal char; actually output */
#define	BS_CONTROL	2	/* \b treated as control char; prints as ^H */

/* How should we search? */
#define	SRCH_FORW       (1 << 0)  /* Search forward from current position */
#define	SRCH_BACK       (1 << 1)  /* Search backward from current position */
#define SRCH_NO_MOVE    (1 << 2)  /* Highlight, but don't move */
#define SRCH_FIND_ALL   (1 << 4)  /* Find and highlight all matches */
#define SRCH_NO_MATCH   (1 << 8)  /* Search for non-matching lines */
#define SRCH_PAST_EOF   (1 << 9)  /* Search past end-of-file, into next file */
#define SRCH_FIRST_FILE (1 << 10) /* Search starting at the first file */
#define SRCH_NO_REGEX   (1 << 12) /* Don't use regular expressions */
#define SRCH_FILTER     (1 << 13) /* Search is for '&' (filter) command */
#define SRCH_AFTER_TARGET (1 << 14) /* Start search after the target line */
#define SRCH_WRAP_AROUND (1 << 15) /* Wrap-around search (continue at BOF/EOF) */

#define	SRCH_REVERSE(t)	(((t) & SRCH_FORW) ? \
				(((t) & ~SRCH_FORW) | SRCH_BACK) : \
				(((t) & ~SRCH_BACK) | SRCH_FORW))

/* */
#define	NO_MCA		0
#define	MCA_DONE	1
#define	MCA_MORE	2

#define	CC_OK		0	/* Char was accepted & processed */
#define	CC_QUIT		1	/* Char was a request to abort current cmd */
#define	CC_ERROR	2	/* Char could not be accepted due to error */
#define	CC_PASS		3	/* Char was rejected (internal) */

#define CF_QUIT_ON_ERASE 0001   /* Abort cmd if its entirely erased */

/* Special char bit-flags used to tell put_line() to do something special */
#define	AT_NORMAL	(0)
#define	AT_UNDERLINE	(1 << 0)
#define	AT_BOLD		(1 << 1)
#define	AT_BLINK	(1 << 2)
#define	AT_STANDOUT	(1 << 3)
#define	AT_ANSI		(1 << 4)  /* Content-supplied "ANSI" escape sequence */
#define	AT_BINARY	(1 << 5)  /* LESS*BINFMT representation */
#define	AT_HILITE	(1 << 6)  /* Internal highlights (e.g., for search) */

/* filestate flags */
#define	CH_CANSEEK	001
#define	CH_KEEPOPEN	002
#define	CH_POPENED	004
#define	CH_HELPFILE	010
#define	CH_NODATA  	020	/* Special case for zero length files */
#define	ch_zero()	((POSITION)0)

/* Flags for cvt_text */
#define	CVT_TO_LC	01	/* Convert upper-case to lower-case */
#define	CVT_BS		02	/* Do backspace processing */
#define	CVT_CRLF	04	/* Remove CR after LF */
#define	CVT_ANSI	010	/* Remove ANSI escape sequences */

/* X11 mouse reporting definitions */
#define X11MOUSE_BUTTON1    0 /* Left button press */
#define X11MOUSE_BUTTON2    1 /* Middle button press */
#define X11MOUSE_BUTTON3    2 /* Right button press */
#define X11MOUSE_BUTTON_REL 3 /* Button release */
#define X11MOUSE_WHEEL_UP   0x40 /* Wheel scroll up */
#define X11MOUSE_WHEEL_DOWN 0x41 /* Wheel scroll down */
#define X11MOUSE_OFFSET     0x20 /* Added to button & pos bytes to create a char */

#define	S_INTERRUPT	01
#define	S_STOP		02
#define S_WINCH		04
#define	ABORT_SIGS()	(sigs & (S_INTERRUPT|S_STOP))

#define	QUIT_OK		0
#define	QUIT_ERROR	1
#define	QUIT_INTERRUPT	2
#define	QUIT_SAVED_STATUS (-1)

#define FOLLOW_DESC     0
#define FOLLOW_NAME     1

#define	OPT_OFF		0
#define	OPT_ON		1
#define	OPT_ONPLUS	2

#define	BAD_LSEEK	((off_t)-1)

/*
 * Flags for open()
 */
#if !UNIX || OS2
#   define	OPEN_READ	(O_RDONLY|O_BINARY)
#elif defined(_OSK)
#   define	OPEN_READ	(S_IREAD)
#elif defined(O_RDONLY)
#   define	OPEN_READ	(O_RDONLY)
#else
#   define	OPEN_READ	(0)
#endif

#if defined(O_WRONLY) && defined(O_APPEND)
#   define	OPEN_APPEND	(O_APPEND|O_WRONLY)
#elif defined(_OSK)
#   define OPEN_APPEND	(S_IWRITE)
#else
#   define	OPEN_APPEND	(1)
#endif

#define	CSI		((unsigned char)'\233')
#define	CHAR_END_COMMAND 0x40000000
#define IS_CSI_START(c)	(((LWCHAR)(c)) == ESC || (((LWCHAR)(c)) == CSI))

/*
 * Set a file descriptor to binary mode.
 */
#if !UNIX || OS2
#define	SET_BINARY(f)	setmode(f, O_BINARY)
#else
#define	SET_BINARY(f)
#endif

/*
 * Does the shell treat "?" as a metacharacter?
 */
#if !UNIX || OS2 || _OSK
#define	SHELL_META_QUEST 0
#else
#define	SHELL_META_QUEST 1
#endif

#define	SPACES_IN_FILENAMES 1

#define	FAKE_HELPFILE	"@/\\less/\\help/\\file/\\@"
#define FAKE_EMPTYFILE	"@/\\less/\\empty/\\file/\\@"

/* ANSI states */
#define ANSI_MID    1
#define ANSI_ERR    2
#define ANSI_END    3

#endif
/*@@copyright@@*/

#define NEWBOT 1

/*
 * Standard include file for "less".
 */

/*
 * Possible values for LESS_PLATFORM.
 * When targeting UNIX, LESS_PLATFORM must be 0
 */
#define LP_UNIX 0
#define LP_DOS_DJGPPC 1
#define LP_DOS_MSC 2
#define LP_DOS_BORLAND 2
#define LP_WINDOWS_BORLAND 4

#define	LP_WINDOWS 100	/* Windows (MSVC) */


/*
 * Include the file of compile-time options.
 * The <> make cc search for it in -I., not srcdir.
 */
#include <defines.h>

#ifdef _SEQUENT_
/*
 * Kludge for Sequent Dynix systems that have sigsetmask, but
 * it's not compatible with the way less calls it.
 * {{ Do other systems need this? }}
 */
#undef HAVE_SIGSETMASK
#endif

/* Library function declarations */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_STDIO_H
#include <stdio.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_CTYPE_H
#include <ctype.h>
#endif
#if HAVE_WCTYPE_H
#include <wctype.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif

/* OS-specific includes */
#ifdef _OSK
#include <modes.h>
#include <strings.h>
#endif

#ifdef __TANDEM
#include <floss.h>
#endif

#if LESS_PLATFORM==LP_WINDOWS || OS2
#include <io.h>
#endif

#if !HAVE_STDLIB_H
char *getenv();
off_t lseek();
void* calloc();
void free();
#endif

/*
 * Simple lowercase test which can be used during option processing
 * (before options are parsed which might tell us what charset to use).
 */
#define ASCII_IS_UPPER(c)	((c) >= 'A' && (c) <= 'Z')
#define ASCII_IS_LOWER(c)	((c) >= 'a' && (c) <= 'z')
#define	ASCII_TO_UPPER(c)	((c) - 'a' + 'A')
#define	ASCII_TO_LOWER(c)	((c) - 'A' + 'a')

#undef IS_UPPER
#undef IS_LOWER
#undef TO_UPPER
#undef TO_LOWER
#undef IS_SPACE
#undef IS_DIGIT

#if HAVE_WCTYPE
#define	IS_UPPER(c)	iswupper(c)
#define	IS_LOWER(c)	iswlower(c)
#define	TO_UPPER(c)	towupper(c)
#define	TO_LOWER(c)	towlower(c)
#elif HAVE_UPPER_LOWER
#define	IS_UPPER(c)	isupper((unsigned char) (c))
#define	IS_LOWER(c)	islower((unsigned char) (c))
#define	TO_UPPER(c)	toupper((unsigned char) (c))
#define	TO_LOWER(c)	tolower((unsigned char) (c))
#else
#define	IS_UPPER(c)	ASCII_IS_UPPER(c)
#define	IS_LOWER(c)	ASCII_IS_LOWER(c)
#define	TO_UPPER(c)	ASCII_TO_UPPER(c)
#define	TO_LOWER(c)	ASCII_TO_LOWER(c)
#endif

#if HAVE_CTYPE_H
#define IS_SPACE(c)	isspace((unsigned char)(c))
#define IS_DIGIT(c)	isdigit((unsigned char)(c))
#else
#define IS_SPACE(c)	((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f')
#define IS_DIGIT(c)	((c) >= '0' && (c) <= '9')
#endif

#define IS_CSI_START(c)	(((LWCHAR)(c)) == ESC || (((LWCHAR)(c)) == CSI))

#include <stdbool.h>

#define	OPT_OFF		0
#define	OPT_ON		1
#define	OPT_ONPLUS	2

#if !HAVE_MEMCPY
#ifndef memcpy
#define	memcpy(to,from,len)	bcopy((from),(to),(len))
#endif
#endif

#if !HAVE_SNPRINTF
/* Use unsafe sprintf if we don't have snprintf. */
#define snprintf(str, size, fmt, ...)	sprintf((str), (fmt), __VA_ARGS__)
#endif

#define	BAD_LSEEK	((off_t)-1)

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/*
 * Upper bound on the string length of an integer converted to string.
 * 302 / 1000 is ceil (log10 (2.0)).  Subtract 1 for the sign bit;
 * add 1 for integer division truncation; add 1 more for a minus sign.
 */
#define INT_STRLEN_BOUND(t) ((sizeof(t) * CHAR_BIT - 1) * 302 / 1000 + 1 + 1)

/*
 * Special types and constants.
 */
typedef unsigned long LWCHAR;
typedef off_t		POSITION;
typedef off_t		LINENUM;
#define MIN_LINENUM_WIDTH  7	/* Min printing width of a line number */
#define MAX_UTF_CHAR_LEN   6	/* Max bytes in one UTF-8 char */

#define	NULL_POSITION	((POSITION)(-1))

/*
 * Flags for open()
 */
#if LESS_PLATFORM || OS2
#define	OPEN_READ	(O_RDONLY|O_BINARY)
#elif defined(_OSK)
#define	OPEN_READ	(S_IREAD)
#elif defined(O_RDONLY)
#define	OPEN_READ	(O_RDONLY)
#else
#define	OPEN_READ	(0)
#endif

#if defined(O_WRONLY) && defined(O_APPEND)
#define	OPEN_APPEND	(O_APPEND|O_WRONLY)
#elif defined(_OSK)
#define OPEN_APPEND	(S_IWRITE)
#else
#define	OPEN_APPEND	(1)
#endif

/*
 * Set a file descriptor to binary mode.
 */
#if LESS_PLATFORM || OS2
#define	SET_BINARY(f)	setmode(f, O_BINARY)
#else
#define	SET_BINARY(f)
#endif

/*
 * Does the shell treat "?" as a metacharacter?
 */
#if LESS_PLATFORM || OS2 || _OSK
#define	SHELL_META_QUEST 0
#else
#define	SHELL_META_QUEST 1
#endif

#define	SPACES_IN_FILENAMES 1

/*
 * An IFILE represents an input file.
 */
#define	IFILE		void*
#define	NULL_IFILE	((IFILE)NULL)

/*
 * The structure used to represent a "screen position".
 * This consists of a file position, and a screen line number.
 * The meaning is that the line starting at the given file
 * position is displayed on the ln-th line of the screen.
 * (Screen lines before ln are empty.)
 */
struct scrpos
{
	POSITION pos;
	int ln;
};

typedef union parg
{
	char *p_string;
	int p_int;
	LINENUM p_linenum;
} PARG;

#define	NULL_PARG	((PARG *)NULL)

struct textlist
{
	char *string;
	char *endstring;
};

struct wchar_range
{
	LWCHAR first, last;
};

struct wchar_range_table 
{
	struct wchar_range *table;
	int count;
};

#define	EOI		(-1)

#define	READ_INTR	(-2)

/* A fraction is represented by an int n; the fraction is n/NUM_FRAC_DENOM */
#define NUM_FRAC_DENOM			1000000
#define NUM_LOG_FRAC_DENOM		6

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

#if '0' == 240
#define IS_EBCDIC_HOST 1
#endif

#if IS_EBCDIC_HOST
/*
 * Long definition for EBCDIC.
 * Since the argument is usually a constant, this macro normally compiles
 * into a constant.
 */
#define CONTROL(c) ( \
	(c)=='[' ? '\047' : \
	(c)=='a' ? '\001' : \
	(c)=='b' ? '\002' : \
	(c)=='c' ? '\003' : \
	(c)=='d' ? '\067' : \
	(c)=='e' ? '\055' : \
	(c)=='f' ? '\056' : \
	(c)=='g' ? '\057' : \
	(c)=='h' ? '\026' : \
	(c)=='i' ? '\005' : \
	(c)=='j' ? '\025' : \
	(c)=='k' ? '\013' : \
	(c)=='l' ? '\014' : \
	(c)=='m' ? '\015' : \
	(c)=='n' ? '\016' : \
	(c)=='o' ? '\017' : \
	(c)=='p' ? '\020' : \
	(c)=='q' ? '\021' : \
	(c)=='r' ? '\022' : \
	(c)=='s' ? '\023' : \
	(c)=='t' ? '\074' : \
	(c)=='u' ? '\075' : \
	(c)=='v' ? '\062' : \
	(c)=='w' ? '\046' : \
	(c)=='x' ? '\030' : \
	(c)=='y' ? '\031' : \
	(c)=='z' ? '\077' : \
	(c)=='A' ? '\001' : \
	(c)=='B' ? '\002' : \
	(c)=='C' ? '\003' : \
	(c)=='D' ? '\067' : \
	(c)=='E' ? '\055' : \
	(c)=='F' ? '\056' : \
	(c)=='G' ? '\057' : \
	(c)=='H' ? '\026' : \
	(c)=='I' ? '\005' : \
	(c)=='J' ? '\025' : \
	(c)=='K' ? '\013' : \
	(c)=='L' ? '\014' : \
	(c)=='M' ? '\015' : \
	(c)=='N' ? '\016' : \
	(c)=='O' ? '\017' : \
	(c)=='P' ? '\020' : \
	(c)=='Q' ? '\021' : \
	(c)=='R' ? '\022' : \
	(c)=='S' ? '\023' : \
	(c)=='T' ? '\074' : \
	(c)=='U' ? '\075' : \
	(c)=='V' ? '\062' : \
	(c)=='W' ? '\046' : \
	(c)=='X' ? '\030' : \
	(c)=='Y' ? '\031' : \
	(c)=='Z' ? '\077' : \
	(c)=='|' ? '\031' : \
	(c)=='\\' ? '\034' : \
	(c)=='^' ? '\036' : \
	(c)&077)
#else
#define	CONTROL(c)	((c)&037)
#endif /* IS_EBCDIC_HOST */

#define	ESC		CONTROL('[')
#define	ESCS		"\33"
#define	CSI		((unsigned char)'\233')
#define	CHAR_END_COMMAND 0x40000000

#if _OSK_MWC32
#define	LSIGNAL(sig,func)	os9_signal(sig,func)
#else
#define	LSIGNAL(sig,func)	signal(sig,func)
#endif

#if HAVE_SIGPROCMASK && !HAVE_SIGSET_T
#undef HAVE_SIGPROCMASK
#endif

#if HAVE_SIGPROCMASK && !HAVE_SIGEMPTYSET
#undef  sigemptyset
#define sigemptyset(mp) *(mp) = 0
#endif

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

/* filestate flags */
#define	CH_CANSEEK	001
#define	CH_KEEPOPEN	002
#define	CH_POPENED	004
#define	CH_HELPFILE	010
#define	CH_NODATA  	020	/* Special case for zero length files */

#define	ch_zero()	((POSITION)0)

#define	FAKE_HELPFILE	"@/\\less/\\help/\\file/\\@"
#define FAKE_EMPTYFILE	"@/\\less/\\empty/\\file/\\@"

/* Flags for cvt_text */
#define	CVT_TO_LC	01	/* Convert upper-case to lower-case */
#define	CVT_BS		02	/* Do backspace processing */
#define	CVT_CRLF	04	/* Remove CR after LF */
#define	CVT_ANSI	010	/* Remove ANSI escape sequences */

#if HAVE_TIME_T
#define time_type	time_t
#else
#define	time_type	long
#endif

/* X11 mouse reporting definitions */
#define X11MOUSE_BUTTON1    0 /* Left button press */
#define X11MOUSE_BUTTON2    1 /* Middle button press */
#define X11MOUSE_BUTTON3    2 /* Right button press */
#define X11MOUSE_BUTTON_REL 3 /* Button release */
#define X11MOUSE_WHEEL_UP   0x40 /* Wheel scroll up */
#define X11MOUSE_WHEEL_DOWN 0x41 /* Wheel scroll down */
#define X11MOUSE_OFFSET     0x20 /* Added to button & pos bytes to create a char */

struct mlist;
struct loption;
struct hilite_tree;
#include "pattern.h"
#include "functions.h"

/* Functions not included in funcs.h */
void postoa (POSITION, char*);
void linenumtoa (LINENUM, char*);
void inttoa (int, char*);
int lstrtoi (char* str, char** end_ptr);
POSITION lstrtopos (char* str, char** end_ptr);

#if LESS_PLATFORM==LP_WINDOWS
int pclose(FILE*);
#endif
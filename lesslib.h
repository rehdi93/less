/*@@copyright@@*/

/*
 * Includes std. library and OS functions.
 * Defines types used by less
 */

#include "lessdef.h"

#ifndef LESSLIB_H
#define LESSLIB_H

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
#if HAVE_STDBOOL_H
#include <stdbool.h>
#endif

/* OS-specific includes */
#ifdef _OSK
#include <modes.h>
#include <strings.h>
#endif

#ifdef __TANDEM
#include <floss.h>
#endif

#if defined(WIN32) || OS2
#include <io.h>
#endif

typedef unsigned long LWCHAR;
typedef off_t		POSITION;
typedef off_t		LINENUM;

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

struct mlist;
struct loption;
struct hilite_tree;
struct ansi_state;

/*
 * An IFILE represents an input file.
 */
#define	IFILE		void*
#define	NULL_IFILE	((IFILE)NULL)

/* Polyfills. */
#if !HAVE_STDLIB_H
char *getenv();
off_t lseek();
void* calloc();
void free();
void* realloc();
#endif
#if !HAVE_STDBOOL_H
enum boolean { false, true };
typedef enum boolean bool;
#endif

/*
 * Simple lowercase test which can be used during option processing
 * (before options are parsed which might tell us what charset to use).
 */
#define ASCII_IS_UPPER(c)	((c) >= 'A' && (c) <= 'Z')
#define ASCII_IS_LOWER(c)	((c) >= 'a' && (c) <= 'z')
#define	ASCII_TO_UPPER(c)	((c) - 'a' + 'A')
#define	ASCII_TO_LOWER(c)	((c) - 'A' + 'a')

#if HAVE_WCTYPE
#define	IS_UPPER(c)	iswupper((wint_t)(c))
#define	IS_LOWER(c)	iswlower((wint_t)(c))
#define	TO_UPPER(c)	towupper((wint_t)(c))
#define	TO_LOWER(c)	towlower((wint_t)(c))
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

#if defined(isspace) || HAVE_CTYPE_H
#define IS_SPACE(c)	isspace((unsigned char)(c))
#else
#define IS_SPACE(c)	((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f')
#endif

#if defined(isdigit) || HAVE_CTYPE_H
#define IS_DIGIT(c)	isdigit((unsigned char)(c))
#else
#define IS_DIGIT(c)	((c) >= '0' && (c) <= '9')
#endif

#if !HAVE_MEMCPY
#ifndef memcpy
#define	memcpy(to,from,len)	bcopy((from),(to),(len))
#endif
#endif

#if !HAVE_SNPRINTF
/* Use unsafe sprintf if we don't have snprintf. */
#define snprintf(str, size, fmt, ...)	sprintf((str), (fmt), __VA_ARGS__)
#endif

#if _OSK_MWC32
int os9_signal (int type, RETSIGTYPE (*handler)());
#define	LSIGNAL(sig,func)	os9_signal(sig,func)
#else
#define	LSIGNAL(sig,func)	signal(sig,func)
#endif

#ifdef _SEQUENT_
/*
 * Kludge for Sequent Dynix systems that have sigsetmask, but
 * it's not compatible with the way less calls it.
 * {{ Do other systems need this? }}
 */
#undef HAVE_SIGSETMASK
#endif

#if HAVE_SIGPROCMASK && !HAVE_SIGSET_T
#undef HAVE_SIGPROCMASK
#endif

#if HAVE_SIGPROCMASK && !HAVE_SIGEMPTYSET
#undef  sigemptyset
#define sigemptyset(mp) *(mp) = 0
#endif

#if !HAVE_TIME_T
#define	time_t	long
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#endif // LESSLIB_H

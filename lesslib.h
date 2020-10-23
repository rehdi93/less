/*@@copyright@@*/

/*
 * Includes std. library and OS functions.
 * Defines types used by less
 */

#ifndef SECURE
#include "defines.h"
#endif

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

#if LESS_PLATFORM==LP_WINDOWS || OS2
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

/*
 * An IFILE represents an input file.
 */
#define	IFILE		void*
#define	NULL_IFILE	((IFILE)NULL)

#endif // LESSLIB_H

/*
 * Polyfill.
 */
#include "lesspoly.h"
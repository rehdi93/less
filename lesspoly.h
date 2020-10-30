/*@@copyright@@*/

/*
 * Include file for polyfills.
 */
#ifndef SECURE
#include "defines.h"
#endif

#ifndef LESSPOLY_H
#define LESSPOLY_H

#if !HAVE_STDLIB_H
char *getenv();
off_t lseek();
void* calloc();
void free();
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

#endif // LESSPOLY_H
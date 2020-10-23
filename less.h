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

#include "lesslib.h"
#include "lessdef.h"

/*
 * Flags for open()
 */
#if LESS_PLATFORM || OS2
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

#define	FAKE_HELPFILE	"@/\\less/\\help/\\file/\\@"
#define FAKE_EMPTYFILE	"@/\\less/\\empty/\\file/\\@"

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
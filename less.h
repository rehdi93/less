/*@@copyright@@*/

/*
 * Standard include file for "less".
 */

#include "lessdef.h"
#include "lesslib.h"

#include "pattern.h"
#include "functions.h"

/* Functions not included in funcs.h */
void postoa (POSITION, char*);
void linenumtoa (LINENUM, char*);
void inttoa (int, char*);
int lstrtoi (char* str, char** end_ptr);
POSITION lstrtopos (char* str, char** end_ptr);

#if defined(WIN32)
int pclose(FILE*);
#endif

void error_(const char* msg);
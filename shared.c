/*@@copyright@@*/

// Shared routines bettween the tools

#include "lesslib.h"

/*
 * Skip leading spaces in a string.
 */
char* skipsp(char *s)
{
	while (*s == ' ' || *s == '\t')	
		s++;
	return s;
}


#ifdef WIN32
/*
  If there is no HOME environment variable, use USERPROFILE.
  If somehow USERPROFILE not set, use the concatenation of HOMEDRIVE + HOMEPATH.
*/
char* win32_get_home()
{
   char* home = getenv("USERPROFILE");
   if (home != NULL) {
       return home;
   }
   else {
        char *drive = getenv("HOMEDRIVE");
        char *path  = getenv("HOMEPATH");
        if (drive != NULL && path != NULL) {
            home = (char*) calloc(strlen(drive) + strlen(path) + 1, sizeof(char));
            strcpy(home, drive);
            strcat(home, path);
        }
        return home;
   }
}

#endif
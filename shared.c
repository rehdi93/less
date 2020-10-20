/*@@copyright@@*/

// Shared routines bettween the tools

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>

#ifdef WIN32

/*
  If there is no HOME environment variable, use USERPROFILE.
  If somehow USERPROFILE not set, use the concatenation of HOMEDRIVE + HOMEPATH.
*/
char* win32_get_home(void*(*alloc)(size_t,size_t))
{
   char* home = getenv("USERPROFILE");
   if (home != NULL) {
       return home;
   }
   else {
        char *drive = getenv("HOMEDRIVE");
        char *path  = getenv("HOMEPATH");
        if (drive != NULL && path != NULL) {
            home = (char*) alloc(strlen(drive) + strlen(path) + 1, sizeof(char));
            strcpy(home, drive);
            strcat(home, path);
        }
        return home;
   }
}

#endif
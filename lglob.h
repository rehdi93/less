/*@@copyright@@*/


/*
 * Macros to define the method of doing filename "globbing".
 * There are three possible mechanisms:
 *   1.	GLOB_LIST
 *	This defines a function that returns a list of matching filenames.
 *   2. GLOB_NAME
 *	This defines a function that steps thru the list of matching
 *	filenames, returning one name each time it is called.
 *   3. GLOB_STRING
 *	This defines a function that returns the complete list of
 *	matching filenames as a single space-separated string.
 */

#if OS2

#define	DECL_GLOB_LIST(list)		char **list;  char **pp;
#define	GLOB_LIST(filename,list)	list = _fnexplode(filename)
#define	GLOB_LIST_FAILED(list)		list == NULL
#define	SCAN_GLOB_LIST(list,p)		pp = list;  *pp != NULL;  pp++
#define	INIT_GLOB_LIST(list,p)		p = *pp
#define	GLOB_LIST_DONE(list)		_fnexplodefree(list)

#elif LESS_PLATFORM==WIN32C && (defined(_MSC_VER) || defined(MINGW))

#define	GLOB_FIRST_NAME(filename,fndp,h) h = _findfirst(filename, fndp)
#define	GLOB_FIRST_FAILED(handle)	((handle) == -1)
#define	GLOB_NEXT_NAME(handle,fndp)	_findnext(handle, fndp)
#define	GLOB_NAME_DONE(handle)		_findclose(handle)
#define	GLOB_NAME			name
#define	DECL_GLOB_NAME(fnd,drive,dir,fname,ext,handle) \
					struct _finddata_t fnd;	\
					char drive[_MAX_DRIVE];	\
					char dir[_MAX_DIR];	\
					char fname[_MAX_FNAME];	\
					char ext[_MAX_EXT];	\
					intptr_t handle;
#endif

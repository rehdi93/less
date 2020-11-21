/*@@copyright@@*/

/*
 * helper functions for "globbing".
 * 
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


struct lglob_s;

void lglob_init(struct lglob_s* g, const char* filename);
bool lglob_failed(struct lglob_s* glob);
bool lglob_next(struct lglob_s* glob);
void lglob_done(struct lglob_s* glob);

#if defined(WIN32) && (defined(_MSC_VER) || defined(MINGW))
#define GLOB_NAME

struct lglob_s
{
	struct _finddata_t find_data;
	intptr_t handle;

// public
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR]; 
	char fname[_MAX_FNAME]; char ext[_MAX_EXT];

	const char* current;
};

// lglob_s has to be an out param, otherwise g.current doesn't work
inline void lglob_init(struct lglob_s* g, const char* filename)
{
	g->handle = _findfirst(filename, &g->find_data);
	g->current = g->find_data.name;
}

inline bool lglob_failed(struct lglob_s* glob) { return glob->handle == -1; }
inline void lglob_done(struct lglob_s* glob) { _findclose(glob->handle); }

inline bool lglob_next(struct lglob_s* g)
{
	return _findnext(g->handle, &g->find_data) == 0;
}


#elif OS2
#define	GLOB_LIST

struct lglob_s
{
	char **list;
	char **iter;

// public
	char *current;
};

inline void lglob_init(struct lglob_s* g, const char* filename)
{
	g->current = NULL;
	g->list = _fnexplode(filename);
	g->iter = g->list;
}

inline bool lglob_failed(struct lglob_s* glob) { return glob->list == NULL; }

inline bool lglob_next(struct lglob_s* glob)
{
	if (glob->iter != NULL) {
		glob->current = *glob->iter;
		glob->iter++;
		return true;
	}
	else {
		glob->iter = glob->list;
		glob->current = NULL;
		return false;
	}
}

inline void lglob_done(struct lglob_s* glob)
{
	_fnexplodefree(glob->list);
}

#elif DOS_DJGPPC
#define GLOB_LIST

struct lglob_s
{
	glob_t list;
	int i;
	
// public
	char *current;
};

inline void lglob_init(struct lglob_s* g, const char* filename)
{
	glob(filename, GLOB_NOCHECK, 0, &g->list);
	g->i = 0;
	g->current = NULL;
}

inline bool lglob_failed(struct lglob_s*) { return false; }

inline bool lglob_next(struct lglob_s* g)
{
	if (g->i < g->list.gl_pathc) {
		g->current = g->list.gl_pathv[glob->i++];
		return true;
	}
	else {
		g->i = 0;
		g->current = NULL;
		return false;
	}
}

inline void lglob_done(struct lglob_s* g)
{
	globfree(&g->list);
}
#elif DOS_PLATFORM
#define GLOB_NAME

struct lglob_s
{
	struct find_t find_data;
	unsigned handle;

// public
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR];
	char fname[_MAX_FNAME]; char ext[_MAX_EXT];

	char* current;
};

inline void lglob_init(struct lglob_s* g, const char* filename)
{
	g->handle = _dos_findfirst(filename, ~_A_VOLID, &g->find_data);
}

inline bool lglob_failed(struct lglob_s* glob) { return glob->handle != 0; }
inline void lglob_done(struct lglob_s*) {}

inline bool lglob_next(struct lglob_s* glob)
{
	glob->handle = _dos_findnext(glob->find_data);
	return glob->handle == 0;
}
#elif WINDOWS_BORLAND
#define GLOB_NAME

struct lglob_s
{
	struct ffblk find_data;
	int handle;

// public
	char drive[MAXDRIVE]; char dir[MAXDIR];
	char fname[MAXFILE]; char ext[MAXEXT];

	char* current;
};

inline void lglob_init(struct lglob_s* g, const char* filename)
{
	g->handle = findfirst(filename, &g->find_data);
	g->current = g->find_data.ff_name;
}
inline bool lglob_failed(struct lglob_s* glob) { return glob->handle != 0; }
inline bool lglob_next(struct lglob_s* glob)
{
	return findnext(glob->handle, &glob->find_data) == 0;
}
inline void lglob_done(struct lglob_s*) {}

#endif

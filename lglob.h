/*@@copyright@@*/

/*
 Common interface for file "globbing".

 'struct lglob_s' stores all the necessary data to call the OS file globbing functions.
 Which is done by the 'lglob_*' functions.
 
 There are three possible mechanisms, depending on which the members of 'struct lglob_s' change:
 1.	GLOB_LIST
	The globbing function returns a list of names.

 struct lglob_s {
	char* current;  the current file
 }

 2.	GLOB_NAME
 	The globbing function returns a single name, and
 	is called multiple times to walk thru all names. (like 'findfirst', 'findnext')

 struct lglob_s {
	const char* current;  the current file name

	char drive[]; char dir[];  buffers to hold the result of _splitpath.
	char fname[]; char ext[];
 }

 3.	GLOB_STRING
	This defines a function that returns the complete list of
	matching filenames as a single space-separated string.

 not implemented.
 */
struct lglob_s;

bool lglob_init(struct lglob_s* g, const char* filename);
bool lglob_next(struct lglob_s* glob);
void lglob_done(struct lglob_s* glob);

#if defined(WIN32) && (defined(_MSC_VER) || defined(MINGW))
#define GLOB_NAME

typedef struct _finddata_t finddata_t;

struct lglob_s
{
	finddata_t find_data;
	intptr_t handle;

// public
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR]; 
	char fname[_MAX_FNAME]; char ext[_MAX_EXT];

	const char* current;
};

// lglob_s has to be an out param, otherwise g.current doesn't work
bool lglob_init(struct lglob_s* g, const char* filename)
{
	g->handle = _findfirst(filename, &g->find_data);
	g->current = g->find_data.name;
	return g->handle != -1;
}

void lglob_done(struct lglob_s* glob) { _findclose(glob->handle); }

bool lglob_next(struct lglob_s* g)
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

bool lglob_init(struct lglob_s* g, const char* filename)
{
	g->current = NULL;
	g->list = _fnexplode(filename);
	g->iter = g->list;
	return g->list != NULL;
}

bool lglob_next(struct lglob_s* glob)
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

void lglob_done(struct lglob_s* glob)
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

bool lglob_init(struct lglob_s* g, const char* filename)
{
	glob(filename, GLOB_NOCHECK, 0, &g->list);
	g->i = 0;
	g->current = NULL;
	return true;
}

bool lglob_next(struct lglob_s* g)
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

void lglob_done(struct lglob_s* g)
{
	globfree(&g->list);
}
#elif DOS_PLATFORM
#define GLOB_NAME

typedef struct find_t finddata_t;

struct lglob_s
{
	finddata_t find_data;

// public
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR];
	char fname[_MAX_FNAME]; char ext[_MAX_EXT];

	char* current;
};

bool lglob_init(struct lglob_s* g, const char* filename)
{
	unsigned result = _dos_findfirst(filename, ~_A_VOLID, &g->find_data);
	g->current = g->find_data.name;
	return result == 0;
}

void lglob_done(struct lglob_s* g) { (void)g; }

bool lglob_next(struct lglob_s* glob)
{
	return _dos_findnext(glob->find_data) == 0;
}
#elif WINDOWS_BORLAND
#define GLOB_NAME

typedef struct ffblk finddata_t;

struct lglob_s
{
	struct ffblk find_data;

// public
	char drive[MAXDRIVE]; char dir[MAXDIR];
	char fname[MAXFILE]; char ext[MAXEXT];

	char* current;
};

bool lglob_init(struct lglob_s* g, const char* filename)
{
	int result = findfirst(filename, &g->find_data);
	g->current = g->find_data.ff_name;
	return result != -1;
}
bool lglob_next(struct lglob_s* glob)
{
	return findnext(&glob->find_data) != -1;
}
void lglob_done(struct lglob_s*) {}

#endif

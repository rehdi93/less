/*
    functions.h

    This replaces the old perl-generated funcs.h
*/

#include <stddef.h>

/* main.c */
char * save (const char *s);
void* ecalloc (size_t count, size_t size);
char * skipsp (char *s);
int sprefix (char *ps, char *s, int uppercase);
void quit (int status);

/* screen.c */
void raw_mode (int on);
void scrsize (void);
char * special_key_str (int key);
void get_term (void);
void init_mouse (void);
void deinit_mouse (void);
void init (void);
void deinit (void);
void home (void);
void add_line (void);
void remove_top (int n);
void win32_scroll_up (int n);
void lower_left (void);
void line_left (void);
void line_left (void);
void check_winch (void);
void goto_line (int sindex);
void vbell (void);
void bell (void);
void clear (void);
void clear_eol (void);
void clear_bot (void);
void at_enter (int attr);
void at_exit (void);
void at_switch (int attr);
int is_at_equiv (int attr1, int attr2);
int apply_at_specials (int attr);
void backspace (void);
void putbs (void);
int win32_kbhit (void);
char WIN32getch (void);
void WIN32setcolors (int fg, int bg);
void WIN32textout (char *text, int len);

/* brac.c */
void match_brac (int obrac, int cbrac, int forwdir, int n);

/* ch.c */
void ch_ungetchar (int c);
void end_logfile (void);
void sync_logfile (void);
int ch_seek (POSITION pos);
int ch_end_seek (void);
int ch_end_buffer_seek (void);
int ch_beg_seek (void);
POSITION ch_length (void);
POSITION ch_tell (void);
int ch_forw_get (void);
int ch_back_get (void);
void ch_setbufspace (int bufspace);
void ch_flush (void);
int seekable (int f);
void ch_set_eof (void);
void ch_init (int f, int flags);
void ch_close (void);
int ch_getflags (void);
void ch_dump (void);

/* charset.c */
void setfmt (char *s, char **fmtvarptr, int *attrptr, char *default_fmt);
void init_charset (void);
int binary_char (LWCHAR c);
int control_char (LWCHAR c);
char * prchar (LWCHAR c);
char * prutfchar (LWCHAR ch);
int utf_len (int ch);
int is_utf8_well_formed (char *ss, int slen);
void utf_skip_to_lead (char **pp, char *limit);
LWCHAR get_wchar (const char *p);
void put_wchar (char **pp, LWCHAR ch);
LWCHAR step_char (char **pp, signed int dir, const char *limit);
int is_composing_char (LWCHAR ch);
int is_ubin_char (LWCHAR ch);
int is_wide_char (LWCHAR ch);
int is_combining_char (LWCHAR ch1, LWCHAR ch2);

/* cmdbuf.c */
void cmd_reset (void);
void clear_cmd (void);
void cmd_putstr (const char *s);
int len_cmdbuf (void);
void set_mlist (void *mlist, int cmdflags);
void cmd_addhist (struct mlist *mlist, const char *cmd, int modified);
void cmd_accept (void);
int cmd_char (int c);
LINENUM cmd_int (long *frac);
char * get_cmdbuf (void);
char * cmd_lastpattern (void);
void init_cmdhist (void);
void save_cmdhist (void);

/* command.c */
int in_mca (void);
void dispversion (void);
int getcc (void);
void ungetcc (LWCHAR c);
void ungetsc (char *s);
LWCHAR peekcc (void);
void commands (void);

/* cvt.c */
int cvt_length (int len, int ops);
int * cvt_alloc_chpos (int len);
void cvt_text (char *odst, char *osrc, int *chpos, int *lenp, int ops);

/* decode.c */
void expand_cmd_tables (void);
void init_cmds (void);
void add_fcmd_table (char *buf, int len);
void add_ecmd_table (char *buf, int len);
int fcmd_decode (char *cmd, char **sp);
int ecmd_decode (char *cmd, char **sp);
char * lgetenv (char *var);
int isnullenv (char* s);
int lesskey (char *filename, int sysvar);
void add_hometable (char *envname, char *def_filename, int sysvar);
int editchar (int c, int flags);

/* edit.c */
void init_textlist (struct textlist *tlist, char *str);
char * forw_textlist (struct textlist *tlist, char *prev);
char * back_textlist (struct textlist *tlist, char *prev);
int edit (char *filename);
int edit_ifile (IFILE ifile);
int edit_list (char *filelist);
int edit_first (void);
int edit_last (void);
int edit_next (int n);
int edit_prev (int n);
int edit_index (int n);
IFILE save_curr_ifile (void);
void unsave_ifile (IFILE save_ifile);
void reedit_ifile (IFILE save_ifile);
void reopen_curr_ifile (void);
int edit_stdin (void);
void cat_file (void);
void use_logfile (char *filename);

/* filename.c */
char * shell_unquote (char *str);
char * get_meta_escape (void);
char * shell_quote (char *s);
char * homefile (char *filename);
char * fexpand (char *s);
char * fcomplete (char *s);
int bin_file (int f);
char * lglob (char *filename);
char * lrealpath (char *path);
char * open_altfile (char *filename, int *pf, void **pfd);
void close_altfile (char *altfilename, char *filename);
int is_dir (char *filename);
char * bad_file (char *filename);
POSITION filesize (int f);
char * shell_coption (void);
char * last_component (char *name);

/* forwback.c */
int eof_displayed (void);
int entire_file_displayed (void);
void squish_check (void);
void forw (int n, POSITION pos, int force, int only_last, int nblank);
void back (int n, POSITION pos, int force, int only_last);
void forward (int n, int force, int only_last);
void backward (int n, int force, int only_last);
int get_back_scroll (void);
int get_one_screen (void);

/* ifile.c */
void del_ifile (IFILE h);
IFILE next_ifile (IFILE h);
IFILE prev_ifile (IFILE h);
IFILE getoff_ifile (IFILE ifile);
int nifile (void);
IFILE get_ifile (char *filename, IFILE prev);
char *get_real_filename(IFILE ifile);
char * get_filename (IFILE ifile);
int get_index (IFILE ifile);
void store_pos (IFILE ifile, struct scrpos *scrpos);
void get_pos (IFILE ifile, struct scrpos *scrpos);
void set_open (IFILE ifile);
int opened (IFILE ifile);
void hold_ifile (IFILE ifile, int incr);
int held_ifile (IFILE ifile);
void * get_filestate (IFILE ifile);
void set_filestate (IFILE ifile, void *filestate);
void set_altpipe (IFILE ifile, void *p);
void * get_altpipe (IFILE ifile);
void set_altfilename (IFILE ifile, char *altfilename);
char * get_altfilename (IFILE ifile);
void if_dump (void);

/* input.c */
POSITION forw_line (POSITION curr_pos);
POSITION back_line (POSITION curr_pos);
void set_attnpos (POSITION pos);

/* jump.c */
void jump_forw (void);
void jump_forw_buffered (void);
void jump_back (LINENUM linenum);
void repaint (void);
void jump_percent (int percent, long fraction);
void jump_line_loc (POSITION pos, int sline);
void jump_loc (POSITION pos, int sline);

/* line.c */
void init_line (void);
int is_ascii_char (LWCHAR ch);
void prewind (void);
void plinenum (POSITION pos);
void pshift_all (void);
int is_ansi_end (LWCHAR ch);
int is_ansi_middle (LWCHAR ch);
void skip_ansi (char **pp, const char *limit);
int pappend (int c, POSITION pos);
int pflushmbc (void);
void pdone (int endline, int chopped, int forw);
void set_status_col (int c);
int gline (int i, int *ap);
void null_line (void);
POSITION forw_raw_line (POSITION curr_pos, char **linep, int *line_lenp);
POSITION back_raw_line (POSITION curr_pos, char **linep, int *line_lenp);
int rrshift (void);

/* linenum.c */
void clr_linenum (void);
void add_lnum (LINENUM linenum, POSITION pos);
LINENUM find_linenum (POSITION pos);
POSITION find_pos (LINENUM linenum);
LINENUM currline (int where);

/* lsystem.c */
void lsystem (char *cmd, char *donemsg);
int pipe_mark (int c, char *cmd);
int pipe_data (char *cmd, POSITION spos, POSITION epos);

/* mark.c */
void init_mark (void);
int badmark (int c);
void setmark (int c, int where);
void clrmark (int c);
void lastmark (void);
void gomark (int c);
POSITION markpos (int c);
char posmark (POSITION pos);
void unmark (IFILE ifile);
void mark_check_ifile (IFILE ifile);
void save_marks (FILE *fout, char *hdr);
void restore_mark (char *line);

/* optfunc.c */
void opt_o (int type, char *s);
void opt__O (int type, char *s);
void opt_j (int type, char *s);
void calc_jump_sline (void);
void opt_shift (int type, char *s);
void calc_shift_count (void);
void opt_k (int type, char *s);
void opt_t (int type, char *s);
void opt__T (int type, char *s);
void opt_p (int type, char *s);
void opt__P (int type, char *s);
void opt_b (int type, char *s);
void opt_i (int type, char *s);
void opt__V (int type, char *s);
void opt_D (int type, char *s);
void opt_x (int type, char *s);
void opt_quote (int type, char *s);
void opt_rscroll (int type, char *s);
void opt_query (int type, char *s);
void opt_mousecap (int type, char *s);
void opt_wheel_lines (int type, char *s);
char * opt_toggle_disallowed(int c);
int get_swindow (void);

/* option.c */
char * propt (int c);
void scan_option (char *s);
void toggle_option (struct loption *o, int lower, char *s, int how_toggle);
int opt_has_param (struct loption *o);
char * opt_prompt (struct loption *o);
int isoptpending (void);
void nopendopt (void);
int getnum (char **sp, char *printopt, int *errp);
long getfraction (char **sp, char *printopt, int *errp);
int get_quit_at_eof (void);

/* opttbl.c */
void init_option (void);
struct loption * findopt (int c);
struct loption * findopt_name (char **p_optname, char **p_oname, int *p_err);

/* os.c */
int iread (int fd, unsigned char *buf, unsigned int len);
void intread (void);
time_t get_time (void);
char * errno_message (char *filename);
int percentage (POSITION num, POSITION den);
POSITION percent_pos (POSITION pos, int percent, long fraction);

/* output.c */
void put_line (void);
void flush (void);
int putchr (int c);
void putstr (const char *s);
void get_return (void);
void error (char *fmt, PARG *parg);
void ierror (char *fmt, PARG *parg);
int query (char *fmt, PARG *parg);

/* pattern.c */
int compile_pattern (char *pattern, int search_type, PATTERN_TYPE *comp_pattern);
void uncompile_pattern (PATTERN_TYPE *pattern);
int valid_pattern (char *pattern);
int is_null_pattern (PATTERN_TYPE pattern);
int match_pattern (PATTERN_TYPE pattern, char *tpattern, char *line, int line_len, char **sp, char **ep, int notbol, int search_type);
char * pattern_lib_name (void);

/* position.c */
POSITION position (int sindex);
void add_forw_pos (POSITION pos);
void add_back_pos (POSITION pos);
void pos_clear (void);
void pos_init (void);
int onscreen (POSITION pos);
int empty_screen (void);
int empty_lines (int s, int e);
void get_scrpos (struct scrpos *scrpos, int where);
int sindex_from_sline (int sline);

/* prompt.c */
void init_prompt (void);
char * pr_expand (const char *proto, int maxwidth);
char * eq_message (void);
char * pr_string (void);
char * wait_message (void);

/* search.c */
void init_search (void);
void repaint_hilite (int on);
void clear_attn (void);
void undo_search (void);
void clr_hlist (struct hilite_tree *anchor);
void clr_hilite (void);
void clr_filter (void);
int is_filtered (POSITION pos);
POSITION next_unfiltered (POSITION pos);
POSITION prev_unfiltered (POSITION pos);
int is_hilited (POSITION pos, POSITION epos, int nohide, int *p_matches);
void chg_hilite (void);
void chg_caseless (void);
int search (int search_type, char *pattern, int n);
void prep_hilite (POSITION spos, POSITION epos, int maxlines);
void set_filter_pattern (char *pattern, int search_type);
int is_filtering (void);

/* signal.c */
RETSIGTYPE winch (int type);
void init_signals (int on);
void psignals (void);

/* tags.c */
void cleantags (void);
int gettagtype (void);
void findtag (char *tag);
POSITION tagsearch (void);
char * nexttag (int n);
char * prevtag (int n);
int ntags (void);
int curr_tag (void);
int edit_tagfile (void);

/* ttyin.c */
void open_getchr (void);
void close_getchr (void);
int default_wheel_lines (void);
int getchr (void);

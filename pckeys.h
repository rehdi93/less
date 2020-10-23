/*@@copyright@@*/


/*
 * Definitions of keys on the PC.
 * Special (non-ASCII) keys on the PC send a two-byte sequence,
 * where the first byte is 0 and the second is as defined below.
 */
#define	PCK_SHIFT_TAB		'\017'
#define	PCK_ALT_E		'\022'
#define	PCK_CAPS_LOCK		'\072'
#define	PCK_F1			'\073'
#define	PCK_NUM_LOCK		'\105'
#define	PCK_HOME		'\107'
#define	PCK_UP			'\110'
#define	PCK_PAGEUP		'\111'
#define	PCK_LEFT		'\113'
#define	PCK_RIGHT		'\115'
#define	PCK_END			'\117'
#define	PCK_DOWN		'\120'
#define	PCK_PAGEDOWN		'\121'
#define	PCK_INSERT		'\122'
#define	PCK_DELETE		'\123'
#define	PCK_CTL_LEFT		'\163'
#define	PCK_CTL_RIGHT		'\164'
#define	PCK_CTL_DELETE		'\223'

/* Definitions of CONTROL,ESC,CSI */
#if '0' == 240
#define IS_EBCDIC_HOST 1
#endif

#if !IS_EBCDIC_HOST
#define	CONTROL(c)	((c)&037)
#else
/*
 * Long definition for EBCDIC.
 * Since the argument is usually a constant, this macro normally compiles
 * into a constant.
 */
#define CONTROL(c) ( \
	(c)=='[' ? '\047' : \
	(c)=='a' ? '\001' : \
	(c)=='b' ? '\002' : \
	(c)=='c' ? '\003' : \
	(c)=='d' ? '\067' : \
	(c)=='e' ? '\055' : \
	(c)=='f' ? '\056' : \
	(c)=='g' ? '\057' : \
	(c)=='h' ? '\026' : \
	(c)=='i' ? '\005' : \
	(c)=='j' ? '\025' : \
	(c)=='k' ? '\013' : \
	(c)=='l' ? '\014' : \
	(c)=='m' ? '\015' : \
	(c)=='n' ? '\016' : \
	(c)=='o' ? '\017' : \
	(c)=='p' ? '\020' : \
	(c)=='q' ? '\021' : \
	(c)=='r' ? '\022' : \
	(c)=='s' ? '\023' : \
	(c)=='t' ? '\074' : \
	(c)=='u' ? '\075' : \
	(c)=='v' ? '\062' : \
	(c)=='w' ? '\046' : \
	(c)=='x' ? '\030' : \
	(c)=='y' ? '\031' : \
	(c)=='z' ? '\077' : \
	(c)=='A' ? '\001' : \
	(c)=='B' ? '\002' : \
	(c)=='C' ? '\003' : \
	(c)=='D' ? '\067' : \
	(c)=='E' ? '\055' : \
	(c)=='F' ? '\056' : \
	(c)=='G' ? '\057' : \
	(c)=='H' ? '\026' : \
	(c)=='I' ? '\005' : \
	(c)=='J' ? '\025' : \
	(c)=='K' ? '\013' : \
	(c)=='L' ? '\014' : \
	(c)=='M' ? '\015' : \
	(c)=='N' ? '\016' : \
	(c)=='O' ? '\017' : \
	(c)=='P' ? '\020' : \
	(c)=='Q' ? '\021' : \
	(c)=='R' ? '\022' : \
	(c)=='S' ? '\023' : \
	(c)=='T' ? '\074' : \
	(c)=='U' ? '\075' : \
	(c)=='V' ? '\062' : \
	(c)=='W' ? '\046' : \
	(c)=='X' ? '\030' : \
	(c)=='Y' ? '\031' : \
	(c)=='Z' ? '\077' : \
	(c)=='|' ? '\031' : \
	(c)=='\\' ? '\034' : \
	(c)=='^' ? '\036' : \
	(c)&077)
#endif /* !IS_EBCDIC_HOST */

#define	ESC		CONTROL('[')
#define	ESCS		"\33"
#define	CSI		((unsigned char)'\233')

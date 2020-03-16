/*@@copyright@@*/


/*
 * Routines dealing with getting input from the keyboard (i.e. from the user).
 */

#include "less.h"

#if OS2
#include "cmd.h"
#include "pckeys.h"
#endif

#if LESS_PLATFORM==WIN32C
#include "os_windows_defs.h"
#define _WIN32_WINNT 0x500
#include <windows.h>
static DWORD console_mode;
HANDLE tty;
#else
int tty;
#endif

extern int sigs;
extern int utf_mode;
extern int wheel_lines;

/*
 * Open keyboard for input.
 */
	void
open_getchr()
{
#if LESS_PLATFORM==WIN32C
	/* Need this to let child processes inherit our console handle */
	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = true;
	tty = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, &sa, 
			OPEN_EXISTING, 0L, NULL);
	GetConsoleMode(tty, &console_mode);
	/* Make sure we get Ctrl+C events. */
	SetConsoleMode(tty, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
#elif LESS_PLATFORM
	extern int fd0;
	/*
	 * Open a new handle to CON: in binary mode 
	 * for unbuffered keyboard read.
	 */
	 fd0 = dup(0);
	 close(0);
	 tty = open("CON", OPEN_READ);
#else
	/*
	 * Try /dev/tty.
	 * If that doesn't work, use file descriptor 2,
	 * which in Unix is usually attached to the screen,
	 * but also usually lets you read from the keyboard.
	 */
#if OS2
	/* The __open() system call translates "/dev/tty" to "con". */
	tty = __open("/dev/tty", OPEN_READ);
#else
	tty = open("/dev/tty", OPEN_READ);
#endif
	if (tty < 0)
		tty = 2;
#endif
}

/*
 * Close the keyboard.
 */
	void
close_getchr()
{
#if LESS_PLATFORM==WIN32C
	SetConsoleMode(tty, console_mode);
	CloseHandle(tty);
#endif
}

#if LESS_PLATFORM==WIN32C
/*
 * Close the pipe, restoring the keyboard (CMD resets it, losing the mouse).
 */
	int
pclose(f)
	FILE *f;
{
	int result;

	result = _pclose(f);
	SetConsoleMode(tty, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
	return result;
}
#endif

/*
 * Get the number of lines to scroll when mouse wheel is moved.
 */
	int
default_wheel_lines()
{
	int lines = 1;
#if LESS_PLATFORM==WIN32C
	if (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0))
	{
		if (lines == WHEEL_PAGESCROLL)
			lines = 3;
	}
#endif
	return lines;
}

/*
 * Get a character from the keyboard.
 */
	int
getchr()
{
	char c;
	int result;

	do
	{
#if LESS_PLATFORM
		/*
		 * In raw read, we don't see ^C so look here for it.
		 */
		flush();
#if LESS_PLATFORM==WIN32C
		if (ABORT_SIGS())
			return (READ_INTR);
		c = WIN32getch();
#else
		c = getch();
#endif
		result = 1;
		if (c == '\003')
			return (READ_INTR);
#else
		{
			unsigned char uc;
			result = iread(tty, &uc, sizeof(char));
			c = (char) uc;
		}
		if (result == READ_INTR)
			return (READ_INTR);
		if (result < 0)
		{
			/*
			 * Don't call error() here,
			 * because error calls getchr!
			 */
			quit(QUIT_ERROR);
		}
#endif
#if 0 /* allow entering arbitrary hex chars for testing */
		/* ctrl-A followed by two hex chars makes a byte */
	{
		static int hex_in = 0;
		static int hex_value = 0;
		if (c == CONTROL('A'))
		{
			hex_in = 2;
			result = 0;
			continue;
		}
		if (hex_in > 0)
		{
			int v;
			if (c >= '0' && c <= '9')
				v = c - '0';
			else if (c >= 'a' && c <= 'f')
				v = c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				v = c - 'A' + 10;
			else
				v = 0;
			hex_value = (hex_value << 4) | v;
			if (--hex_in > 0)
			{
				result = 0;
				continue;
			}
			c = hex_value;
		}
	}
#endif
		/*
		 * Various parts of the program cannot handle
		 * an input character of '\0'.
		 * If a '\0' was actually typed, convert it to '\340' here.
		 */
		if (c == '\0')
			c = '\340';
	} while (result != 1);

	return (c & 0xFF);
}

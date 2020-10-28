/*@@copyright@@*/


/*
 * lessecho [-ox] [-cx] [-pn] [-dn] [-a] file ...
 * Simply echos its filename arguments on standard output.
 * But any argument containing spaces is enclosed in quotes.
 *
 * -ox	Specifies "x" to be the open quote character.
 * -cx	Specifies "x" to be the close quote character.
 * -pn	Specifies "n" to be the open quote character, as an integer.
 * -dn	Specifies "n" to be the close quote character, as an integer.
 * -mx  Specifies "x" to be a metachar.
 * -nn  Specifies "n" to be a metachar, as an integer.
 * -ex  Specifies "x" to be the escape char for metachars.
 * -fn  Specifies "x" to be the escape char for metachars, as an integer.
 * -a	Specifies that all arguments are to be quoted.
 *	The default is that only arguments containing spaces are quoted.
 */

#include "lesslib.h"

static char *version = "$Revision: 1.15 $";

static int quote_all = 0;
static char openquote = '"';
static char closequote = '"';
static char *meta_escape = "\\";
static char meta_escape_buf[2];
static char metachars[64] = "";
static int num_metachars = 0;

static void pr_usage()
{
	fprintf(stderr,
		"usage: lessecho [-ox] [-cx] [-pn] [-dn] [-mx] [-nn] [-ex] [-fn] [-a] file ...\n");
}

static void pr_version()
{
	char *p;
	char buf[10];
	char *pbuf = buf;

	for (p = version;  *p != ' ';  p++)
		if (*p == '\0')
			return;
	for (p++;  *p != '$' && *p != ' ' && *p != '\0';  p++)
		*pbuf++ = *p;
	*pbuf = '\0';
	printf("%s\n", buf);
}

static void pr_error(char* s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}


#if !HAVE_STRCHR
char * strchr(s, c)
	char *s;
	int c;
{
	for ( ;  *s != '\0';  s++)
		if (*s == c)
			return (s);
	if (c == '\0')
		return (s);
	return (NULL);
}
#endif

int main(int argc, char* argv[])
{
	char *arg;
	char *s;
	int no_more_options;

	no_more_options = 0;
	while (--argc > 0)
	{
		arg = *++argv;
		if (*arg != '-' || no_more_options)
			break;
		switch (*++arg)
		{
		case 'a':
			quote_all = 1;
			break;
		case 'c':
			closequote = *++arg;
			break;
		case 'd':
			closequote = (char)strtol(++arg, &s, 10);
			if (s == arg)
				pr_error("Missing number after -d");
			break;
		case 'e':
			if (strcmp(++arg, "-") == 0)
				meta_escape = "";
			else
				meta_escape = arg;
			break;
		case 'f':
			meta_escape_buf[0] = (char)strtol(++arg, &s, 10);
			meta_escape = meta_escape_buf;
			if (s == arg)
				pr_error("Missing number after -f");
			break;
		case 'o':
			openquote = *++arg;
			break;
		case 'p':
			openquote = (char)strtol(++arg, &s, 10);
			if (s == arg)
				pr_error("Missing number after -p");
			break;
		case 'm':
			metachars[num_metachars++] = *++arg;
			metachars[num_metachars] = '\0';
			break;
		case 'n':
			metachars[num_metachars++] = (char)strtol(++arg, &s, 10);
			if (s == arg)
				pr_error("Missing number after -n");
			metachars[num_metachars] = '\0';
			break;
		case '?':
			pr_usage();
			return (0);
		case '-':
			if (*++arg == '\0')
			{
				no_more_options = 1;
				break;
			}
			if (strcmp(arg, "version") == 0)
			{
				pr_version();
				return (0);
			}
			if (strcmp(arg, "help") == 0)
			{
				pr_usage();
				return (0);
			}
			pr_error("Invalid option after --");
		default:
			pr_error("Invalid option letter");
		}
	}

	while (argc-- > 0)
	{
		int has_meta = 0;
		arg = *argv++;
		for (s = arg;  *s != '\0';  s++)
		{
			if (strchr(metachars, *s) != NULL)
			{
				has_meta = 1;
				break;
			}
		}
		if (quote_all || (has_meta && strlen(meta_escape) == 0))
			printf("%c%s%c", openquote, arg, closequote);
		else 
		{
			for (s = arg;  *s != '\0';  s++)
			{
				if (strchr(metachars, *s) != NULL)
					printf("%s", meta_escape);
				printf("%c", *s);
			}
		}
		if (argc > 0)
			printf(" ");
		else
			printf("\n");
	}
	return (0);
}

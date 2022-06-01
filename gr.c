/* date: 30.05.2022 */

/*

LS(1)                       General Commands Manual                      LS(1)

SYNOPSIS
     gr pattern [-cnwgi] [file ...]

DESCRIPTION
     gr is a pattern searching program in spirit similar
     to grep, but different in syntax and options.
     gr is simpler, and focuses on fast workflow,
     not completeness in features.

           [   beginning of line
           ]   end of line
           ?   any character
           %   any word characters
           ~   any non word characters

OPTIONS
     -c   caseless matching
     -n   print file and line number
     -w   show warnings
     -g   print with pretty colors
     -i   use stdin instead of file

ENVIRONMENT
     FILECOLOR  determine a program for printing filenames
                when -g and -n are used

*/

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <err.h>


struct {
    unsigned caseless :1;
    unsigned warn     :1;
    unsigned line     :1;
    unsigned color    :1;
    unsigned stream   :1;
    unsigned havefile :1;
} flag = {0};

char *ls = "";
char *fname  = NULL;
char *fcolor = NULL;


void
printfname(char *name)
{
	char c, cm[512];
	FILE *f;
	unsigned i;
	if (!strcmp(name, "-"))
		return (void)fputs("<stdin>", stdout);
	if (!*ls || strlen(name) + strlen(ls) + 2 > sizeof(cm))
		return (void)fputs(name, stdout);
	if (fname == name && fcolor)
		return (void)fputs(fcolor, stdout);
	else fname = name;
	sprintf(cm, "%s %s", ls, name);
	f = popen(cm, "r");
	for (i = 0; (c = getc(f)) > 0; i++) {
		if (c == '\n') break;
		if (!(i % 20)) fcolor = realloc(fcolor, i+20);
		fcolor[i] = c;
		putc(c, stdout);
	};
	if (!(i % 20)) fcolor = realloc(fcolor, i+20);
	fcolor[i] = 0;
	fclose(f);
}


int
word(unsigned c)
{
	return (c >= 'A' && c <= 'Z')
	||     (c >= 'a' && c <= 'z');
}

int
nonword(unsigned c)
{
	return !word(c) && c >= ' ';
}


unsigned
chc(unsigned c)
{
	if (!flag.caseless) return c;
	else return c + (c <= 'Z' && c >= 'A' ? ('a' - 'A') : 0);
}

unsigned
wd(unsigned a, unsigned b)
{
	return nonword(a) && word(b);
}


unsigned
match(char *e, char *s, unsigned i)
{
	char *t, *esc = "ntarf\0E" "\n\t\a\r\f";
	/* There is a bug here where '{' is present after '}'
	 * this should not be a problem (will not fix). */
	for (; s[i]; i++, e++) {
		switch (*e) {
		case '\\': if (!*++e) return i;
			if ((t = strchr(esc, *e))) *e = t[strlen(esc)+1];
		default: if (chc(*e) != chc(s[i])) return 0; break;
		case '[': if (i != 0) return 0; else i--; break;
		case ']': if (s[i] != '\n' || !s[i]) return 0; break;
		case '%': while (word(s[i]))    i++; i--; break;
		case '~': while (nonword(s[i])) i++; i--; break;
		case '?': break;
		case  0 : return i;
		};
	};
	return *e == 0 ? i : 0;
}


void
grep(char *e, char *name)
{
	FILE *f;
	size_t l;
	unsigned i, j, n;
	char *s;
	if (!strcmp(name, "-")) f = stdin;
	else f = fopen(name, "r");
	if (!f && flag.warn) {
		warnx("cannot open file");
		return;
	};
	for (s = NULL, l = n = 0; 0 < getline(&s, &l, f); n += 1) {
		for (j = i = 0; !i && s[j]; j++) i = match(e, s, j);
		if (!i) continue;
		else j -= 1;
		if (!flag.line && !flag.color) {
			printf("%s", s);
		} else if (!flag.color) {
			printf("%s:%4d:%s", name, n+1, s);
		} else if (!flag.line){
			printf("\033[m%.*s\033[31m%.*s\033[m%s",
			    j, s, i, s+j, s+j+i);
		} else {
			printfname(name);
			printf("\033[38;5;237m:%5d:\033[m%.*s\033[31m%.*s\033[m%s",
			    n+1, j, s, i-j, s+j, s+i);
		};
	};
	fclose(f);
}

void
grall(char *e)
{
	struct dirent **dir = NULL;
	signed n, i;
	n = scandir(".", &dir, NULL, alphasort);
	if (n == -1) err(1, "scandir");
	for (i = 2; i < n; i++) {
		grep(e, dir[i]->d_name);
		free(dir[n]);
	};
	free(dir);
}

void
gecolor()
{
	char *name;
	name = getenv("FILECOLOR");
	if (!name || strlen(name) + 1 > sizeof(ls)) {
		return;
	} else {
		ls = name;
	};
}

void
geflag(char *s)
{
	while (*s) {
		switch(*(s++)) {
		case 'c': flag.caseless = 1; break;
		case 'w': flag.warn     = 1; break;
		case 'n': flag.line     = 1; break;
		case 'i':
			flag.stream   = 1;
			flag.havefile = 1;
			break;
		case 'g':
			if (!flag.color) gecolor();
			flag.color = 1;
			break;
		default:
			errx(1, "invalid option: %c", s[-1]);
		};
	};
}


int
main(int argc, char **argv)
{
	unsigned i;
	char *e = NULL;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') geflag(argv[i]+1);
		else if (!e) e = argv[i];
		else flag.havefile = 1, grep(e, argv[i]);
	};
	if (!e) errx(1, "an argument expected");
	if (!flag.havefile) grall(e);
	if (flag.stream) grep(e, "-");
	return 0;
}




































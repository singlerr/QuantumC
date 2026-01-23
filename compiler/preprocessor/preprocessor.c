#include "preprocessor.h"
#include "filebuf.h"
#include <stdlib.h>
#include <string.h>

#define PARSE_SUCCESS 1
#define PARSE_PASS 0

int getch(FILE *in);
int parse(FILE *in);

int prep_readline(char **buf, int *len, FILE *in)
{
    int c = getch(in);
    if (c == EOF)
    {
        return EOF;
    }

    // read token????

    if (!parse(in))
    {
        *len = getbuf(buf);
        return *len;
    }
}

int getch(FILE *in)
{
    int c = readchar(in);

    if (c == EOF)
    {
        return EOF;
    }

    // reserve code space for future
    return c;
}

static int dir_define(FILE *in)
{
    char *line;
    int len = peekbuf(&line);

    while (!strncmp(line, "define", len))
    {
        if (!strcmp(line, "define"))
        {
            return PARSE_SUCCESS;
        }
        readchar(in);
        len = peekbuf(&line);
    }

    return PARSE_PASS;
}

static int dir_if(FILE *in)
{
    char *line;
    int len = peekbuf(&line);

    while (!strncmp(line, "if", len))
    {
        if (!strcmp(line, "if"))
        {
            return PARSE_SUCCESS;
        }
        readchar(in);
        len = peekbuf(&line);
    }

    return PARSE_PASS;
}

static int directive(FILE *in)
{
    return (dir_define(in) || dir_if(in));
}

static int scan(FILE *in)
{
}

int parse(FILE *in)
{
    return (directive(in) || scan(in));
}
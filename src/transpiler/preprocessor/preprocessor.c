#include "preprocessor.h"
#include "filebuf.h"
#include <stdlib.h>
#include <string.h>

#define NAMELEN 2048

#define PARSE_SUCCESS 1
#define PARSE_PASS 0

int getch(FILE *in);
int parse(FILE *in);

enum macro_chnk_type
{
    TEXT,
    PLACEHOLDER
};

struct macro_args
{
    int count;
    char *name[NAMELEN];
};

struct macro_body_chnk
{
    enum macro_chnk_type chnk_type;
    char *content;
    struct macro_body_chnk *next;
    struct macro_body_chnk *prev;
}

struct macro
{
    int require_args;
    struct macro_args *args;
    char name[NAMELEN];

    struct macro *prev;
};

struct macro *macro_list = NULL;

static struct macro *find_macro(char *name)
{
    struct macro *macro;
    for (macro = macro_list; macro; macro = macro->prev)
    {
        if (!strcmp(macro->name, name))
        {
            return macro;
        }
    }

    return NULL;
}

static void add_macro(struct macro *macro)
{
    macro->prev = macro_list;
    macro_list = macro;
}

int prep_readline(char **buf, size_t *len, FILE *in)
{
    if (!buf_initialized())
    {
        init_buf();
    }

    int c = getch(in);
    if (c == EOF)
    {
        return EOF;
    }

    // read token????

    if (!parse(in))
    {
        *len = (size_t)getbuf(buf);
        return (int)*len;
    }

    return
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

static int dir_define_body(FILE *in)
{
}

static int dir_define_args(FILE *in, struct macro_args **out_args)
{
    char *line;
    int len;
    int c;
    struct macro_args *args;

    skip_whitespace(in);
    c = peekchar();

    // no paren, which means this macro does not have any args
    if (c != '(')
    {
        return dir_define_body(in);
    }

    // next search args
    c = readchar(in);
    skip_whitespace(in);
    c = peekchar();

    // read up to end of paren
    while (1)
    {

        if (c == ')')
        {
            break;
        }
    }
}
static int dir_define(FILE *in)
{
    char *line;
    int len = peekbuf(&line);

    while (!strncmp(line, "define", len))
    {
        if (!strcmp(line, "define"))
        {
            resetbuf();
            return dir_define_args();
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
    char *line;
    struct macro *defined_macro;

    int len;
    len = peekbuf(&line);
    defined_macro = find_macro(line);

    if (defined_macro)
    {
    }
}

int parse(FILE *in)
{
    return (directive(in) || scan(in));
}
#include "preprocessor.h"
#include "filebuf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAMELEN 2048

#define TRUE 1
#define FALSE 0

#define PARSE_SUCCESS 1
#define PARSE_PASS 0

int getch(FILE *in);
int parse(FILE *in);

enum macro_chnk_type
{
    TEXT,
    PLACEHOLDER
};

struct macro_arg
{
    char name[NAMELEN + 1];
};

struct macro_args
{
    int count;
    struct macro_arg *names;
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
    char name[NAMELEN + 1];

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

static void append_body(struct macro_body_chnk **prev, enum macro_chnk_type type, char *content)
{
    struct macro_body_chnk *chnk = (struct macro_body_chnk *)malloc(sizeof(struct macro_body_chnk));
    chnk->content = content;
    chnk->chnk_type = type;

    if (!*prev)
    {
        *prev = chnk;
    }
    else
    {
        (*prev)->next = chnk;
        chnk->prev = (*prev);
    }
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

static int dir_define_body(FILE *in, struct macro *macro)
{

    char *line;
    int len;
    int c;
    c = peekchar();
    while (1)
    {

        if (c == '\\')
        {
            // goto next line
            while (c != '\n')
            {
                c = readchar(in);
            }

            continue;
        }

        if (c == '\n')
        {
            break;
        }

        // split token
        if (isspace(c))
        {
            len = getbuf(&line);
        }

        c = readchar(in);
    }
}

static int dir_define_args(FILE *in, struct macro_args **out_args, struct macro *macro)
{
    char *line;
    int len;
    int c;
    int arg_count;
    struct macro_args *args;

    skip_whitespace(in);
    c = peekchar();

    // no paren, which means this macro does not have any args
    if (c != '(')
    {
        *out_args = NULL;
        macro->require_args = FALSE;
        return dir_define_body(in, macro);
    }

    args = (struct macro_args *)malloc(sizeof(struct macro_args));
    args->count = 0;
    args->names = NULL;

    // next search args
    c = readchar(in);
    skip_whitespace(in);

    arg_count = 0;
    // read up to end of paren
    while (1)
    {
        c = readchar();
        skip_whitespace(in);
        // identifier
        while (isdigit(c) || isalpha(c) || c == '_')
        {
            c = readchar(in);
        }
        skip_whitespace(in);
        if (c == ',' || c == ')')
        {
            // consume current arg
            // and resume
            len = getbuf(&line);

            if (len < 1)
            {
                // empty args
                break;
            }

            if (len >= NAMELEN)
            {
                fprintf(stderr, "Name defined in macro must not be longer than", NAMELEN);
                exit(1);
            }

            if (!args->names)
            {
                args->names = (struct macro_arg *)malloc(sizeof(struct macro_arg));
                args->count++;
                strncpy(args->names->name, line, len);
            }
            else
            {
                args->names = (struct macro_arg *)realloc(sizeof(struct macro_arg) * (args->count + 1));
                strncpy(args->names[args->count++].name, line, len);
            }
            continue;
        }

        // end of args
        if (c == ')')
        {
            readchar(in);
            // begin body
            break;
        }
    }

    *out_args = args;
    return dir_define_body(in, macro);
}
static int dir_define(FILE *in)
{
    char *line;
    int len;
    struct macro_args *args;
    struct macro *macro;

    len = peekbuf(&line);

    while (!strncmp(line, "define", len))
    {
        if (!strcmp(line, "define"))
        {
            macro = (struct macro *)malloc(sizeof(struct macro));
            resetbuf();
            int ret = dir_define_args(in, &args, macro);
            macro->args = args;

            add_macro(macro);
            return ret;
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
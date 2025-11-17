#include "symrec.h"
#include <string.h>
#include <stdlib.h>

struct
{
    symrec_t *curr;
} sym_table;

struct
{
    typrec_t *curr;
} typ_table;

symrec_t *
putsym(const char *name, typrec_t *type, int level)
{
    symrec_t *res = (symrec_t *)malloc(sizeof(symrec_t));
    res->name = strdup(name);
    res->type = type;
    res->prev = sym_table.curr;
    sym_table.curr = res;
    return res;
}

symrec_t *getsym(const char *name, int level)
{
    for (symrec_t *p = sym_table.curr; p; p = p->prev)
    {
        if (p->level < level)
            break;
        if (strcmp(p->name, name) == 0 && p->level == level)
            return p;
    }

    return 0;
}

typrec_t *gettyp(const char *name)
{
    for (typrec_t *p = typ_table.curr; p; p = p->prev)
    {
        if (strcmp(name, p->name) == 0)
        {
            return p;
        }
    }

    return 0;
}

typrec_t *puttyp(const char *name)
{
    typrec_t *p = (typrec_t *)malloc(sizeof(typrec_t));
    p->name = strdup(name);
    p->prev = typ_table.curr;

    typ_table.curr = p;
    return p;
}
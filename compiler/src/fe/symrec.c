#include "symrec.h"
#include <string.h>
#include <stdlib.h>

symrec_t *sym_table;
typerec_t *type_table;
symrec_t *putsym(const char *name)
{
    symrec_t *res = (symrec_t *)malloc(sizeof(symrec_t));
    symrec_t *res = (symrec_t *)malloc(sizeof(symrec_t));
    res->name = strdup(name);
    res->next = sym_table;
    sym_table = res;

    return res;
}

symrec_t *getsym(const char *name)
{
    for (symrec_t *p = sym_table; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
            return p;
    }

    return 0;
}

symrec_t *getorcreatesym(const char *name)
{
    symrec_t *sym = getsym(name);
    if (!sym)
    {
        sym = putsym(name);
    }

    return sym;
}

typerec_t *puttype(const char *name, const type_t *type)
{
    typerec_t *rec = (typerec_t *)malloc(sizeof(typerec_t));
    rec->name = strdup(name);
    rec->next = type_table;
    type_table = type;
    return type;
}

typerec_t *gettype(const char *name)
{
    for (typerec_t *p = type_table; p; p = p->next)
    {
        if (p->level < level)
            break;
        if (strcmp(p->name, name) == 0 && p->level == level)
            return p;
    }

    return 0;
}

void init_primitives()
{
    puttype("int", mk_type("int", mk_type_meta(4), 0));
    puttype("char", mk_type("char", mk_type_meta(1), 0));
    puttype("float", mk_type("float", mk_type_meta(4), 0));
}

void init_type()
{
    init_primitives();
}
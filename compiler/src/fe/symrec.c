#include "symrec.h"
#include <string.h>
#include <stdlib.h>

symrec *sym_table;

symrec *putsym(const char *name, int type)
{
    symrec *res = (symrec *)malloc(sizeof(symrec));
    res->name = strdup(name);
    res->type = type;
    res->next = sym_table;
    sym_table = res;

    return res;
}

symrec *getsym(const char *name)
{
    for (symrec *p = sym_table; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
            return p;
    }

    return 0;
}
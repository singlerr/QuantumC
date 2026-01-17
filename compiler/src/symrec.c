#include "symrec.h"
#include "ast.h"
#include "stringlib.h"
#include <string.h>
#include <stdlib.h>

symrec_t *sym_table;
typerec_t *type_table;

typerec_t *PRIM_VOID;
typerec_t *PRIM_CHAR;
typerec_t *PRIM_SHORT;
typerec_t *PRIM_INT32;
typerec_t *PRIM_INT64;
typerec_t *PRIM_LONG;
typerec_t *PRIM_FLOAT32;
typerec_t *PRIM_FLOAT64;
typerec_t *PRIM_DOUBLE;
typerec_t *PRIM_SIGNED;
typerec_t *PRIM_UNSIGNED;
typerec_t *PRIM_COMPLEX;
typerec_t *PRIM_IMAGINARY;
typerec_t *PRIM_STRING;
typerec_t *PRIM_DURATION;
typerec_t *PRIM_BIT;
typerec_t *PRIM_BOOL;
typerec_t *PRIM_QUBIT;
typerec_t *PRIM_ANGLE;

symrec_t *putsym(const char *name)
{
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

typerec_t *puttype(const char *name, ast_node_type type_type, const type_t *type)
{
    typerec_t *rec = (typerec_t *)malloc(sizeof(typerec_t));
    rec->name = strdup(name);
    rec->next = type_table;
    rec->handle = (type_t *)type;
    rec->type_type = type_type;
    type_table = rec;
    return rec;
}

typerec_t *gettype(const char *name)
{
    for (typerec_t *p = type_table; p; p = p->next)
    {
        if (strcmp(p->name, name) == 0)
            return p;
    }

    return 0;
}

typerec_t *clone_type_rec(const typerec_t *o)
{
    typerec_t *t = (typerec_t *)malloc(sizeof(typerec_t));
    t->name = strdup(o->name);
    t->type_type = o->type_type;
    t->next = o->next;

    type_t *h = clone_type(o->handle);
    t->handle = h;

    return t;
}

void init_primitives()
{
    PRIM_VOID = PUT_TYPE("void", AST_TYPE_VOID, 0);
    PRIM_CHAR = PUT_TYPE("char", AST_TYPE_CHAR, 1);
    PRIM_SHORT = PUT_TYPE("short", AST_TYPE_SHORT, 2);
    PRIM_INT32 = PUT_TYPE("int", AST_TYPE_INT, 4);
    PRIM_INT64 = PUT_TYPE("int64", AST_TYPE_INT, 8);
    PRIM_LONG = PUT_TYPE("long", AST_TYPE_INT, 8);
    PRIM_FLOAT32 = PUT_TYPE("float", AST_TYPE_FLOAT, 4);
    PRIM_FLOAT64 = PUT_TYPE("float64", AST_TYPE_FLOAT, 8);
    PRIM_DOUBLE = PUT_TYPE("double", AST_TYPE_DOUBLE, 4);
    PRIM_SIGNED = PUT_TYPE("signed", AST_TYPE_SIGNED, 4);
    PRIM_UNSIGNED = PUT_TYPE("unsigned", AST_TYPE_UNSIGNED, 4);
    PRIM_COMPLEX = PUT_TYPE("complex", AST_TYPE_COMPLEX, 4);
    PRIM_IMAGINARY = PUT_TYPE("imaginary", AST_TYPE_IMAGINARY, 4);
    PRIM_STRING = PUT_TYPE("string", AST_TYPE_STRING, 4);
    PRIM_DURATION = PUT_TYPE("duration", AST_TYPE_DURATION, 4);
    PRIM_BIT = PUT_TYPE("bit", AST_TYPE_BIT, 1);
    PRIM_BOOL = PUT_TYPE("bool", AST_TYPE_BOOL, 4);
    PRIM_QUBIT = PUT_TYPE("qubit", AST_TYPE_QUBIT, 1);
    PRIM_ANGLE = PUT_TYPE("angle", AST_TYPE_ANGLE, 1);
}

void init_type()
{
    init_primitives();
}
#ifndef _SYMREC_H_
#define _SYMREC_H_

#include "type.h"

typedef struct _symrec
{
    char *name;
    struct _symrec *next;
} symrec_t;

typedef struct _typerec
{
    char *name;
    struct _typerec *next;
} typerec_t;

extern symrec_t *sym_table;
extern typerec_t *type_table;

symrec_t *putsym(const char *name);
symrec_t *getsym(const char *name);
symrec_t *getorcreatesym(const char *name);

typerec_t *puttype(const char *name, const type_t *type);
typerec_t *gettype(const char *name);

#endif
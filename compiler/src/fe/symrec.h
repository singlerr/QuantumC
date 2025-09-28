#ifndef _SYMREC_H_
#define _SYMREC_H_

typedef struct _symrec
{
    char *name;
    int type;

    struct _symrec *next;
} symrec;

extern symrec *sym_table;

symrec *putsym(const char *name, int type);
symrec *getsym(const char *name);

#endif
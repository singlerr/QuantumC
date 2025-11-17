#ifndef _SYMREC_H_
#define _SYMREC_H_

typedef struct _typrec
{
    char *name;
    int type;
    struct _typrec *prev;
} typrec_t;

typedef struct _symrec
{
    char *name;
    typrec_t *type;
    int level;
    struct _symrec *prev;
} symrec_t;

symrec_t *putsym(const char *name, typrec_t *type, int level);
symrec_t *getsym(const char *name, int level);

typrec_t *puttyp(const char *name);
typrec_t *gettyp(const char *name);

#endif
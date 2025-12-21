#ifndef _TYPE_H_
#define _TYPE_H_

typedef struct _typemeta
{
    int size;
} typemeta_t;

typedef struct _type
{
    char *name;
    typemeta_t *meta;
    struct _type *link;
} type_t;

typemeta_t *mk_type_meta(int size);
typemeta_t *clone_type_meta(const typemeta_t *o);
type_t *mk_type(const char *name, const typemeta_t *meta, type_t *link);
type_t *clone_type(const type_t *o);

#endif
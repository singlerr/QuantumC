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
type_t *mk_type(const char *name, const typemeta_t *meta, type_t *link);

#endif
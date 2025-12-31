#include "type.h"
#include "stringlib.h"
#include <stdlib.h>
#include <string.h>
typemeta_t *mk_type_meta(int size)
{
    typemeta_t *meta = (typemeta_t *)malloc(sizeof(typemeta_t));
    meta->size = size;

    return meta;
}
type_t *mk_type(const char *name, const typemeta_t *meta, type_t *link)
{
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->link = link;
    t->meta = (typemeta_t *)meta;
    t->name = strdup(name);
    return t;
}

typemeta_t *clone_type_meta(const typemeta_t *o)
{
    typemeta_t *meta = (typemeta_t *)malloc(sizeof(typemeta_t));
    meta->size = o->size;
    return meta;
}

type_t *clone_type(const type_t *o)
{
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->link = o->link;
    typemeta_t *m = clone_type_meta(o->meta);
    t->meta = m;
    t->name = strdup(o->name);

    return t;
}
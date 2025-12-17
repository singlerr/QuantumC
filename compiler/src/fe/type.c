#include "type.h"
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
    t->meta = meta;
    t->name = strdup(name);
    return t;
}
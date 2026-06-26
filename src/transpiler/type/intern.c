#include "intern.h"
#include "../data/vec/cvector.h"
#include <stdlib.h>
#include <string.h>

static cvector_vector_type (type_t *) intern_table;

/* Structural equality for the intern-table bucket check.
   Compound types rely on pointer identity of their ref fields, so refs must
   already be interned before the parent is interned. */
static int
type_struct_equal (const type_t *a, const type_t *b)
{
  if (a->tag != b->tag || a->size != b->size)
    return 0;
  switch (a->tag)
    {
    case TY_POINTER:
      return a->ty.ty_pointer.ref == b->ty.ty_pointer.ref;
    case TY_ARRAY:
      return a->ty.ty_array.ref == b->ty.ty_array.ref
             && a->ty.ty_array.size == b->ty.ty_array.size;
    case TY_STRUCT:
    case TY_UNION:
      return a->ty.ty_struct.has_name == b->ty.ty_struct.has_name
             && strncmp (a->ty.ty_struct.name, b->ty.ty_struct.name,
                         SYM_MAXLEN)
                    == 0;
    case TY_FUN:
      return 0; /* ponytail: always distinct; interned by identity */
    default:
      return 1; /* primitive: tag + size is enough */
    }
}

type_t *
intern_type (type_t *candidate)
{
  type_t **it;
  if (!candidate)
    return NULL;
  for (it = cvector_begin (intern_table); it != cvector_end (intern_table);
       it++)
    {
      if (type_struct_equal (*it, candidate))
        {
          free (candidate);
          return *it;
        }
    }
  cvector_push_back (intern_table, candidate);
  return candidate;
}

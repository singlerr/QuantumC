#include "tytab.h"
#include "../data/vec/cvector.h"
#include "../err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ty_tab
{
  char name[SYM_MAXLEN];
  type_t *origin;
} ty_tab_t;

typedef cvector_vector_type (ty_tab_t) ty_tabs_t;
ty_tabs_t ty_tabs;

type_t *
find_type_by_name (const char *name)
{
  ty_tab_t *it;

  for (it = cvector_begin (ty_tabs); it != cvector_end (ty_tabs); it++)
    {
      if (!strncmp (it->name, name, SYM_MAXLEN))
        {
          return it->origin;
        }
    }
  return NULL;
}

void
push_type (const char *name, type_t *origin)
{
  ty_tab_t t;
  if (!origin)
    {
      warn ("Could not push null type");
      return;
    }

  strncpy (t.name, name, SYM_MAXLEN);
  t.origin = origin;
  cvector_push_back (ty_tabs, t);
}

void
init_type_table (void)
{
  push_type ("uint8_t",  new_simple_type (1, TY_UINT));
  push_type ("uint16_t", new_simple_type (2, TY_UINT));
  push_type ("uint32_t", new_simple_type (4, TY_UINT));
  push_type ("uint64_t", new_simple_type (8, TY_UINT));
  push_type ("int8_t",   new_simple_type (1, TY_INT));
  push_type ("int16_t",  new_simple_type (2, TY_INT));
  push_type ("int32_t",  new_simple_type (4, TY_INT));
  push_type ("int64_t",  new_simple_type (8, TY_INT));
  push_type ("size_t",   new_simple_type (8, TY_UINT));
  push_type ("qubit",    new_simple_type (1, TY_QUBIT));
}

#include "tytab.h"
#include "data/vec/cvector.h"
#include "err.h"
#include <stdio.h>
#inlc
#include <string.h>

typedef struct ty_tab
{
  char name[SYM_MAXLEN];
  type_t *origin;
} ty_tab_t;

typedef cvector_vector_type (ty_tab_t) ty_tabs_t;
ty_tabs_t ty_tabs;

const type_t *
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
push_type (const char *name, const type_t *origin)
{
  ty_tab_t *t;
  if (!origin)
    {
      warn ("Could not push null type");
      return;
    }

  t = (ty_tab_t *)malloc (sizeof (ty_tab_t));
  strncpy (t->name, name, SYM_MAXLEN);
  t->origin = origin;

  cvector_push_back (ty_tabs, t);
}

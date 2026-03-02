#include "symtab.h"
#include "data/vec/cvector.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cvector_vector_type (symtab_t) symbols;
int scope = 0;
int var_id = 0;

const symtab_t *
search_symbol (const char *name, int only_current_scope)
{
  symtab_t *it;

  for (it = cvector_begin (symbols); it != cvector_end (symbols); it++)
    {
      if (it->scope < scope && only_current_scope)
        {
          return NULL;
        }

      if (!strcmp (it->name, name))
        {
          return it;
        }
    }

  return NULL;
}

const symtab_t *
put_symbol (const char *name, uint32_t id)
{
  symtab_t *s = (symtab_t *)malloc (sizeof (symtab_t));
  s->id = var_id++;
  s->scope = scope;
  strncpy (s->name, name, SYM_MAXLEN);

  return s;
}

void
pop_symbols ()
{
  symtab_t *back = cvector_back (symbols);

  if (!back)
    {
      return;
    }

  if (back->scope > scope)
    {
      error ("Symtab scope must be equal or lower than current scope; "
             "expected: %d, current: %d",
             scope, back->scope);
    }

  if (back->scope < scope)
    {
      // if symtab scope is smaller than current scope, just decrease
      dec_scope ();
      return;
    }

  while (back && back->scope == scope)
    {
      cvector_pop_back (symbols);
      back = cvector_back (symbols);
    }
}

void
inc_scope ()
{
  scope++;
}

void
dec_scope ()
{
  scope--;
}

void
init_symtab ()
{
  symbols = vector_create ();
}
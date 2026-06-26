#include "symtab.h"
#include "data/vec/cvector.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cvector_vector_type (symtab_t) symbols;
int scope = 0;

const symtab_t *
search_symbol (const char *name, int only_current_scope)
{
  symtab_t *it;

  for (it = cvector_begin (symbols); it != cvector_end (symbols); it++)
    {
      if (it->scope < scope && only_current_scope)
        return NULL;

      if (!strcmp (it->name, name))
        return it;
    }

  return NULL;
}

ty_deco_t *
search_symbol_type (const char *name, int only_current_scope)
{
  const symtab_t *sym = search_symbol (name, only_current_scope);
  return sym ? sym->ty : NULL;
}

const symtab_t *
put_symbol (const char *name, ty_deco_t *ty)
{
  symtab_t s;
  s.ty = ty;
  s.scope = scope;
  strncpy (s.name, name, SYM_MAXLEN);
  cvector_push_back (symbols, s);
  return &symbols[cvector_size (symbols) - 1];
}

void
pop_symbols ()
{
  size_t sz = cvector_size (symbols);
  if (sz == 0)
    return;

  while (cvector_size (symbols) > 0
         && symbols[cvector_size (symbols) - 1].scope == scope)
    cvector_pop_back (symbols);

  dec_scope ();
}

void
inc_scope ()
{
  scope++;
}

void
dec_scope ()
{
  if (scope > 0)
    scope--;
}

void
init_symtab ()
{
  symbols = NULL;
}

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "param.h"
#include "type/deco.h"
#include <stdint.h>

typedef struct symtab
{
  char name[SYM_MAXLEN];
  ty_deco_t *ty;
  uint32_t scope;
} symtab_t;

const symtab_t *search_symbol (const char *name, int only_current_scope);
const ty_deco_t *search_symbol_type (const char *name, int only_current_scope);
const symtab_t *put_symbol (const char *name, ty_deco_t *ty);
void pop_symbols ();

void inc_scope ();
void dec_scope ();
void init_symtab ();

#endif
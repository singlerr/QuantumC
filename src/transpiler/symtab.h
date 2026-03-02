#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "param.h"
#include <stdint.h>

typedef struct symtab
{
  char name[SYM_MAXLEN];
  uint32_t id;
  uint32_t scope;
} symtab_t;

const symtab_t *search_symbol (const char *name, int only_current_scope);
const symtab_t *put_symbol (const char *name, uint32_t id);
void pop_symbols ();

void inc_scope ();
void dec_scope ();
void init_symtab ();

#endif
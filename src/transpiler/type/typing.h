#ifndef _TYPING_H_
#define _TYPING_H_

#include "deco.h"

typedef enum subtype_relation
{
  SUBTYPE_RELATION,
  SUPERTYPE_RELATION,
  TYPE_INCOMPATIBLE,
  TYPE_EQUIVALENT,
  COERCION_REQUIRED
} subtype_relation_t;

typedef struct subtype_result
{
  subtype_relation_t relation;
  ty_deco_t *type;
  int requires_cast;
} subtype_result_t;

#endif
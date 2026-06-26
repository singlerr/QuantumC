#ifndef _INTERN_H_
#define _INTERN_H_

#include "base.h"

/* intern_type: return the canonical pointer for a structurally-equal type.
   If an equal type exists in the table, frees candidate and returns it.
   Otherwise registers candidate and returns it. */
type_t *intern_type (type_t *candidate);

/* After interning, structural equality collapses to pointer identity. */
#define type_equals(a, b) ((a) == (b))

#endif

#include <stdio.h>
#include <stdlib.h>
#include "ast_sem.h"
#include "common.h"
#include "builtin_func.h"
#include "builtin_quantum.h"
#include "diagnostics.h"

/* ponytail: stub — full measure conversion in Layer 4 */
BUILTIN_FUNC (measure)
{
  (void)func_call;
  *out = IALLOC (expression);
  (*out)->kind = EXPR_QUANTUM_MEASUREMENT;
}

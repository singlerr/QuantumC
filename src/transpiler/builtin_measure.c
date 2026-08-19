#include "ast_sem.h"
#include "builtin_func.h"
#include "builtin_quantum.h"
#include "common.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

BUILTIN_FUNC (measure)
{
  (void)func_call;
  *out = IALLOC (expression);
  (*out)->kind = EXPR_QUANTUM_MEASUREMENT;
}

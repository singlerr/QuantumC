#include "ast_sem.h"
#include "builtin_func.h"
#include "builtin_quantum.h"
#include "common.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

#define BUILTIN_GATE(GATE)                                                    \
  BUILTIN_FUNC (apply_##GATE)                                                 \
  {                                                                           \
    (void)func_call;                                                          \
    *out = IALLOC (expression);                                               \
    (*out)->kind = EXPR_QUANTUM_GATE;                                         \
  }

BUILTIN_GATE (X)
BUILTIN_GATE (Y)
BUILTIN_GATE (Z)
BUILTIN_GATE (S)
BUILTIN_GATE (T)
BUILTIN_GATE (H)
BUILTIN_GATE (CNOT)
BUILTIN_GATE (CZ)
BUILTIN_GATE (CX)
BUILTIN_GATE (CCX)
BUILTIN_GATE (RX)
BUILTIN_GATE (RY)
BUILTIN_GATE (RZ)

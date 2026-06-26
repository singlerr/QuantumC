#include "ast_sem.h"
#include <stdlib.h>

/* ponytail: synth_expr is called from main.c before convert_program
   to keep type/base.h and ast_sem.h's 'struct type' in separate TUs */
void
convert_program (ast_t *root, program **out)
{
  (void)root;
  *out = calloc (1, sizeof (program));
}

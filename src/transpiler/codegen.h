#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stdio.h>

#include "hir/hir.h"

void set_codegen_output (FILE *out);
void gen_program (hir_program *prog);

#endif

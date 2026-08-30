#ifndef _QSUGAR_H_
#define _QSUGAR_H_

#include "../hir/hir.h"

typedef struct _q_context
{

} q_ctx;

int quantumify (hir_program *ast, q_ctx *ctx);

#endif
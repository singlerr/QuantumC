#include <stdio.h>
#include <stdlib.h>
#include "ast_sqz.h"
#include "ast_sem.h"
#include "common.h"
#include "builtin_func.h"
#include "builtin_quantum.h"
#include "diagnostics.h"

BUILTIN_FUNC(measure)
{
    expression *measure_expr = IALLOC(expression);
    expression_list *arg_list;
    convert_expression_arguments(func_call->args, &arg_list);
    measure_expr->as.quantum_measurement.measure.qubit = IALLOC(qubit);
    expression *qubit = arg_list->value;
    identifier *id;
    if (!qubit)
    {
        P_ERROR("Measure function must take 1 parameter at least");
    }
    measure_expr->as.quantum_measurement.measure.qubit = new_qubit(qubit);
    measure_expr->kind = EXPR_QUANTUM_MEASUREMENT;
    *out = measure_expr;
}
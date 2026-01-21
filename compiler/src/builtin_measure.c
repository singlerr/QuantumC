#include <stdio.h>
#include <stdlib.h>
#include "ast_sqz.h"
#include "ast_sem.h"
#include "common.h"
#include "builtin_func.h"
#include "diagnostics.h"

extern void convert_expression_arguments(const sqz_args *args, expression_list **out);

static inline indexed_identifier *new_indexed_identifier(identifier *identifier, expr_or_range_list *index)
{
    indexed_identifier *id = IALLOC(indexed_identifier);
    index_element *elem = IALLOC(index_element);
    elem->kind = EXPR_EXPRESSION;
    elem->index.expr_or_range = index->value;
    id->name = identifier;
    id->index = elem;

    return id;
}

static inline qubit *new_qubit(expression *expr)
{
    qubit *q = IALLOC(qubit);
    identifier *id;
    switch (expr->kind)
    {
    case EXPR_INDEX:
        if (expr->as.index.collection->kind != EXPR_IDENTIFIER)
        {
            P_ERROR("Expected identifier but not found for measure");
            break;
        }

        id = expr->as.index.collection->as.identifier;
        q->kind = ID_INDEXED_IDENTIFIER;
        q->value.indexed_identifier = new_indexed_identifier(id, expr->as.index.list);
        break;
    case EXPR_IDENTIFIER:
        id = expr->as.identifier;
        q->kind = ID_IDENTIFIER;
        q->value.identifier = id;
        break;
    default:
        P_ERROR("Unsupported parameter type");
        return NULL;
    }

    return q;
}

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
#ifndef _BUILTIN_QUANTUM_H_
#define _BUILTIN_QUANTUM_H_

#include <stdio.h>
#include <stdlib.h>
#include "ast_sem.h"
#include "ast_sqz.h"
#include "common.h"
#include "diagnostics.h"

extern void convert_expression_arguments(const sqz_args *args, expression_list **out);
extern identifier *new_identifier(char *name);
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

#endif
#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXEC_OP
#define WRAP(val) ((val))
#define CONV(val, type) ((type)val)
#define EXEC_OP(__op, __l, __r, __val)                                 \
    do                                                                 \
    {                                                                  \
        if (__l->kind == __r->kind)                                    \
        {                                                              \
            if (__l->kind == OP_INTEGER)                               \
                __val = WRAP(__l->value.i) __op WRAP(__r->value.i);    \
            else if (__l->kind == OP_FLOAT)                            \
                __val = WRAP(__l->value.i) __op WRAP(__r->value.i);    \
        }                                                              \
        else if (__l->kind == OP_INTEGER && __r->kind == OP_FLOAT)     \
        {                                                              \
            __val = CONV(__l->value.i, float) __op WRAP(__r->value.f); \
        }                                                              \
        else if (__l->kind == OP_FLOAT && __r->kind == OP_INTEGER)     \
        {                                                              \
            __val = WRAP(__l->value.f) __op CONV(__r->value.i, float); \
        }                                                              \
    } while (0);
#endif

struct directive *directives = NULL;

static struct placeholder *append_placeholder(struct placeholder *__dest, struct placeholder *__new)
{

    if (!__dest)
    {
        return __new;
    }

    __new->prev = __dest;
    __dest->next = __new;

    return __new;
}

static struct directive *push_directive(struct directive *__dest, struct directive *__new)
{
    if (!__dest)
    {
        return __new;
    }

    __new->prev = __dest;

    return __new;
}

int validate_expr(enum if_op operator, struct operand *l, struct operand *r)
{
    int result;
    switch (operator)
    {
    case IF_L:
        EXEC_OP(<, l, r, result);
        break;
    case IF_G:
        EXEC_OP(>, l, r, result);
        break;
    case IF_LE:
        EXEC_OP(<=, l, r, result);
        break;
    case IF_GE:
        EXEC_OP(>=, l, r, result);
        break;
    case IF_NE:
        EXEC_OP(!=, l, r, result);
        break;
    case IF_EQ:
        EXEC_OP(==, l, r, result);
        break;
    default:
        perror("Unknown operator");
        result = 0;
    }

    free(l);
    free(r);
    return result;
}

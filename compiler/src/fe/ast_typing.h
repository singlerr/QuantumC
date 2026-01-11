#ifndef _AST_TYPING_H_
#define _AST_TYPING_H_

#include "type.h"
#include "common.h"

#ifndef TYPE_OP
#define TYPE_OP

#define IS_PTR(type) (type->meta->node_type == AST_TYPE_POINTER)
#define IS_INT(type) (type == PRIM_INT)
#define IS_VOID(type) (type == PRIM_VOID)
#define IS_CHAR(type) (type == PRIM_CHAR)
#define IS_SHORT(type) (type == PRIM_SHORT)
#define IS_LONG(type) (type == PRIM_LONG)
#define IS_FLOAT(type) (type == PRIM_FLOAT)
#define IS_DOUBLE(type) (type == PRIM_DOUBLE)
#define IS_SIGNED(type) (type == PRIM_SIGNED)
#define IS_UNSIGNED(type) (type == PRIM_UNSIGNED)
#define IS_COMPLEX(type) (type == PRIM_COMPLEX)
#define IS_IMAGINARY(type) (type == PRIM_IMAGINARY)
#define IS_QUBIT(type) (type == PRIM_QUBIT)
#define IS_ANGLE(type) (type == PRIM_ANGLE)
#define IS_ARRAY(type) (type->meta->node_type == AST_TYPE_ARRAY)
#define IS_STRUCT(type) (type->meta->node_type == AST_TYPE_STRUCT)
#define IS_AGGREGATE(type) (IS_ARRAY(type) || IS_STRUCT(type))
#define IS_EXCL_NULL(a, b) ((!a && b) || (a && !b))
#define IS_INCL_NULL(a, b) (!a || !b)
#define SHALLOW_TYPE_EQUAL(a, b) (a == b || a->meta->node_type == b->meta->node_type)
#define SPEC_EQUAL(a, b) (a->qualifier == b->qualifier && a->storage_class == b->storage_class)
#define IS_INTEGRAL(type) (IS_INT(type) || IS_CHAR(type) || IS_SHORT(type) || IS_LONG(type) || IS_SIGNED(type) || IS_UNSIGNED(type))
#define IS_NUMERIC(type) (IS_INTEGRAL(type) || IS_FLOAT(type) || IS_DOUBLE(type))
#define IS_SCALAR(type) (IS_NUMERIC(type) || IS_PTR(type))
#define IS_UNION(type) (type->meta->node_type == AST_TYPE_UNION)
#define IS_FUNC(type) (type->meta->node_type == AST_TYPE_FUNCTION)
#define ARRAY_ACCESSIBLE(type) (IS_ARRAY(type) || IS_PTR(type))
#define ARRAY_INDEXIBLE(type) (IS_INTEGRAL(type))
#endif

BOOL type_equals(const type_t *a, const type_t *b);
BOOL is_type_compatible(const type_t *left, const type_t *right);
BOOL is_casting_compatible(const type_t *caster, const type_t *castee);
BOOL is_param_compatible(const struct sem_args *a, const struct sem_args *b);

#endif
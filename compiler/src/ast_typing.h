#ifndef _AST_TYPING_H_
#define _AST_TYPING_H_

#include "type.h"
#include "common.h"

struct sem_args;
struct _sqz_args;

#ifndef TYPE_OP
#define TYPE_OP

#define NAME_EQUAL(a, b) (strcmp(a->name, b->name) == 0)
#define REC2TYPE(rec) (rec->handle)
#define IS_PTR(type) (type->meta->node_type == AST_TYPE_POINTER)
#define IS_INT32(type) (type == REC2TYPE(PRIM_INT32) || NAME_EQUAL(type, REC2TYPE(PRIM_INT32)))
#define IS_INT64(type) (type == REC2TYPE(PRIM_INT64) || NAME_EQUAL(type, REC2TYPE(PRIM_INT64)))
#define IS_INT(type) (IS_INT32(type) || IS_INT64(type))
#define IS_VOID(type) (type == REC2TYPE(PRIM_VOID) || NAME_EQUAL(type, REC2TYPE(PRIM_VOID)))
#define IS_CHAR(type) (type == REC2TYPE(PRIM_CHAR) || NAME_EQUAL(type, REC2TYPE(PRIM_CHAR)))
#define IS_SHORT(type) (type == REC2TYPE(PRIM_SHORT) || NAME_EQUAL(type, REC2TYPE(PRIM_SHORT)))
#define IS_LONG(type) (type == REC2TYPE(PRIM_LONG) || NAME_EQUAL(type, REC2TYPE(PRIM_LONG)))
#define IS_FLOAT32(type) (type == REC2TYPE(PRIM_FLOAT32) || NAME_EQUAL(type, REC2TYPE(PRIM_FLOAT32)))
#define IS_FLOAT64(type) (type == REC2TYPE(PRIM_FLOAT64) || NAME_EQUAL(type, REC2TYPE(PRIM_FLOAT64)))
#define IS_FLOAT(type) (IS_FLOAT32(type) || IS_FLOAT64(type))
#define IS_DOUBLE(type) (type == REC2TYPE(PRIM_DOUBLE) || NAME_EQUAL(type, REC2TYPE(PRIM_DOUBLE)))
#define IS_SIGNED(type) (type == REC2TYPE(PRIM_SIGNED) || NAME_EQUAL(type, REC2TYPE(PRIM_SIGNED)))
#define IS_UNSIGNED(type) (type == REC2TYPE(PRIM_UNSIGNED) || NAME_EQUAL(type, REC2TYPE(PRIM_UNSIGNED)))
#define IS_COMPLEX(type) (type == REC2TYPE(PRIM_COMPLEX) || NAME_EQUAL(type, REC2TYPE(PRIM_COMPLEX)))
#define IS_IMAGINARY(type) (type == REC2TYPE(PRIM_IMAGINARY) || NAME_EQUAL(type, REC2TYPE(PRIM_IMAGINARY)))
#define IS_QUBIT(type) (type == REC2TYPE(PRIM_QUBIT) || NAME_EQUAL(type, REC2TYPE(PRIM_QUBIT)))
#define IS_ANGLE(type) (type == REC2TYPE(PRIM_ANGLE) || NAME_EQUAL(type, REC2TYPE(PRIM_ANGLE)))
#define IS_DURATION(type) (type == REC2TYPE(PRIM_DURATION) || NAME_EQUAL(type, REC2TYPE(PRIM_DURATION)))
#define IS_BIT(type) (type == REC2TYPE(PRIM_BIT) || NAME_EQUAL(type, REC2TYPE(PRIM_BIT)))
#define IS_BOOL(type) (type == REC2TYPE(PRIM_BOOL) || NAME_EQUAL(type, REC2TYPE(PRIM_BOOL)))
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
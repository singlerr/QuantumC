#ifndef _AST_SQZ_H_
#define _AST_SQZ_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"

typedef struct _sqz_var_decl
{

} sqz_var_decl;

typedef struct _sqz_func_decl
{

} sqz_func_decl;

typedef struct _sqz_primary_expr
{
    ast_node_type primary_type;
    union
    {
        struct _sqz_id *identifier;
        struct _sqz_expr *expr;
        int i;
        float f;
        char *s;
    } value;

} sqz_primary_expr;

typedef struct _sqz_id
{
    ast_node_type id_type;
    symrec_t *name;
    typerec_t *type;
} sqz_id;

struct _sqz_post
{
    ast_node_type op_type;
    struct _sqz_expr_src *operand;
};

typedef struct _sqz_expr
{
    // TODO
    struct _sqz_expr *next;
} sqz_expr;

typedef struct _sqz_args
{
    // TODO
} sqz_args;

typedef struct _sqz_expr_src_struct_init
{
    typerec_t *struct_type;
    // TODO initializer list
} sqz_expr_src_struct_init;

typedef struct _sqz_expr_src_arr_access
{
    struct _sqz_expr_src *array;
    sqz_expr *index;
} sqz_expr_src_arr_access;

typedef struct _sqz_expr_src_func_call
{
    struct _sqz_expr_src *func;
    sqz_args *args;

} sqz_expr_src_func_call;

struct _sqz_post_unary
{
    ast_node_type op_type;
    struct _sqz_expr_src *operand;
};

typedef struct _sqz_expr_src_member_access
{
    ast_node_type access_type;
    struct _sqz_expr_src *owner;
    sqz_id *member_name;

} sqz_expr_src_member_access;

typedef struct _sqz_expr_src
{
    ast_node_type expr_type;

    union
    {
        sqz_expr_src_func_call *func_call;
        sqz_expr_src_member_access *member_access;
        sqz_expr_src_arr_access *arr_access;
        sqz_expr_src_struct_init *struct_init;
        sqz_primary_expr *primary_expr;
    } expr;

} sqz_expr_src;

typedef struct _sqz_cast_expr
{
    type_t *type;

    union
    {
        struct _sqz_cast_expr *cast;
        union _sqz_unary *unary;
    };

} sqz_cast_expr;

typedef union _sqz_unary
{

    ast_node_type expr_type;

    union
    {
        sqz_cast_expr *cast;
        sqz_expr_src *postfix;
        struct _sqz_sizeof *sizeof_expr;
        struct _sqz_pre *pre_inc_dec;
        struct _sqz_pre_unary *pre_unary_op;
    } expr;

} sqz_unary;

struct _sqz_pre
{
    ast_node_type op_type;
    sqz_unary *operand;
};

struct _sqz_pre_unary
{
    ast_node_type op_type;
    sqz_cast_expr *operand;
};

struct _sqz_sizeof
{
    BOOL is_unary_expr;

    union
    {
        sqz_unary *unary;
        type_t *type;
    };
};

typedef struct _sqz_binary_expr
{
    ast_node_type expr_type;
    struct _sqz_binary_expr *left;

    union
    {
        sqz_unary *unary;
        struct _sqz_binary_expr *binary;
    } right;
} sqz_binary_expr;

typedef struct _sqz_ternary_expr
{
    sqz_binary_expr *condition;
    sqz_expr *true_expr;
    struct _sqz_ternary_expr *false_expr;
} sqz_ternary_expr;

typedef struct _sqz_assign_expr
{

} sqz_assign_expr;

typedef struct _sqz_stmt
{

} sqz_stmt;

#endif
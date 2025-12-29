#ifndef _AST_SQZ_H_
#define _AST_SQZ_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"

#define STG_EXTERN (1)
#define STG_STATIC (1 << 1)
#define STG_AUTO (1 << 2)
#define STG_REGISTER (1 << 3)

#define QAL_CONST (1)
#define QAL_RESTRICT (1 << 1)
#define QAL_VOLATILE (1 << 2)

struct _sqz_spec_qual
{
    unsigned int qualifier;
    type_t *type;
    struct _sqz_spec_qual *next;
};

typedef struct _sqz_param_decl
{
    struct _sqz_decl_spec *spec;
    struct _sqz_type *decl;
} sqz_param_decl;

typedef struct _sqz_designator
{
    union
    {
        struct _sqz_id *id;
        struct _sqz_ternary_expr *expr;
    } val;
    ast_node_type designator_type;
    struct _sqz_designator *next;
} sqz_designator;

typedef struct _sqz_type
{
    type_t *type;
    struct _sqz_assign_expr *index;
    struct _sqz_args *args;
    struct _sqz_id *id;
    struct _sqz_spec_qual *qual;
    struct _sqz_type *next;
} sqz_type;

typedef struct _sqz_decl_spec
{
    unsigned int storage_class;
    unsigned int qualifier;
} sqz_decl_spec;

typedef struct _sqz_decl
{
    ast_node_type decl_type;
    union
    {
        struct _sqz_var_decl *var;
        struct _sqz_func_decl *func;
    } decl;

    struct _sqz_decl *next;
} sqz_decl;

typedef struct _sqz_initializer
{
    int level;
    struct _sqz_assign_expr *expr;
    sqz_designator *designator;
    struct _sqz_initializer *next;
} sqz_initializer;

typedef struct _sqz_var_decl
{
    sqz_decl_spec *spec;
    type_t *type;
    symrec_t *symbol;
    sqz_initializer *init;

    struct _sqz_var_decl *next;
} sqz_var_decl;

typedef struct _sqz_func_decl
{
    struct _sqz_decl_spec *spec;
    type_t *return_type;
    struct _sqz_args *params;
    struct _expr_compound_stmt *body;
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
    int scope_level;
    sqz_decl_spec *spec;
    symrec_t *name;
    type_t *type;
} sqz_id;

struct _sqz_post
{
    ast_node_type op_type;
    struct _sqz_expr_src *operand;
};

typedef struct _sqz_expr
{
    struct _sqz_assign_expr *expr;
    struct _sqz_expr *next;
} sqz_expr;

typedef struct _sqz_args
{
    sqz_param_decl *arg;
    struct _sqz_args *next;
} sqz_args;

typedef struct _sqz_expr_src_struct_init
{
    type_t *struct_type;

    // TODO initializer list : will be implemented in the future
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
        struct _sqz_post *post_inc_dec;

    } expr;

} sqz_expr_src;

typedef struct _sqz_cast_expr
{
    type_t *type;

    union
    {
        struct _sqz_cast_expr *cast;
        struct _sqz_unary *unary;
    };

} sqz_cast_expr;

typedef struct _sqz_unary
{

    ast_node_type expr_type;

    union
    {
        sqz_cast_expr *cast;
        sqz_expr_src *postfix;
        struct _sqz_sizeof *sizeof_expr;
        struct _sqz_pre *pre_inc_dec;
        struct _sqz_pre_unary *pre_unary_op;
        sqz_type *type_name;
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
    sqz_cast_expr *cast_expr;
    struct _sqz_binary_expr *left;
    union
    {
        sqz_cast_expr *cast;
        struct _sqz_binary_expr *binary;
    } right;
} sqz_binary_expr;

typedef struct _sqz_ternary_expr
{
    sqz_binary_expr *condition;
    sqz_expr *true_expr;
    struct _sqz_ternary_expr *false_expr;

    sqz_binary_expr *binary_expr;
} sqz_ternary_expr;

typedef struct _sqz_assign_expr
{
    ast_node_type assign_type;
    sqz_unary *left;
    struct _sqz_assign_expr *right;
    sqz_ternary_expr *ternary_expr;
} sqz_assign_expr;

struct sqz_labeled
{
    symrec_t *label;
    struct _sqz_stmt *stmt;
};

struct sqz_case
{
    sqz_ternary_expr *case_expr;
    struct _sqz_stmt *stmt;
};

struct sqz_default
{
    struct _sqz_stmt *stmt;
};

struct _sqz_block_item
{
    ast_node_type decl_or_stmt;
    union
    {
        struct _sqz_var_decl *decl;
        struct _sqz_stmt *stmt;
    } item;

    struct _sqz_block_item *next;
};

struct sqz_compound_stmt
{
    struct _sqz_block_item *block_list;
};

struct sqz_expr_stmt
{
    struct _sqz_expr *expr;
};

struct sqz_if
{
    struct _sqz_expr *expr;
    struct _sqz_stmt *true_stmt;
};

struct sqz_if_else
{
    struct _sqz_expr *expr;
    struct _sqz_stmt *true_stmt;
    struct _sqz_stmt *false_stmt;
};

struct sqz_switch
{
    struct _sqz_expr *expr;
    struct _sqz_stmt *body;
};

struct sqz_while
{
    struct _sqz_expr *expr;
    struct _sqz_stmt *body;
};

struct sqz_do_while
{
    struct _sqz_expr *expr;
    struct _sqz_stmt *body;
};

struct sqz_for
{
    struct _sqz_var_decl *decl;
    struct _sqz_expr_stmt *cond;
    struct _sqz_expr *eval;

    struct _sqz_stmt *body;
};

struct sqz_goto
{
    symrec_t *label;
};

struct sqz_continue
{
};

struct sqz_break
{
};

struct sqz_return
{
    sqz_expr *expr;
};

struct sqz_jump
{
    ast_node_type jump_type;
    union
    {
        struct sqz_goto *goto_stmt;
        struct sqz_continue *continue_stmt;
        struct sqz_break *break_stmt;
        struct sqz_return *return_stmt;
    } jump;
};

struct sqz_iter
{
    ast_node_type iter_type;
    union
    {
        struct sqz_while *while_iter;
        struct sqz_do_while *do_while_iter;
        struct sqz_for *for_iter;
    } iter;
};

struct sqz_selection
{
    ast_node_type type;
    union
    {
        struct sqz_if *if_selection;
        struct sqz_if_else *if_else_selection;
        struct sqz_switch *switch_selection;
    } selection;
};

typedef struct _sqz_stmt
{
    ast_node_type stmt_type;
    union
    {
        struct sqz_compound_stmt *compound;
        struct sqz_expr_stmt *expr;
        struct sqz_labeld *labeled;
        struct sqz_iter *iter;
        struct sqz_jump *jump;
        struct sqz_selection *selection;
    } stmt;

} sqz_stmt;

typedef struct _sqz_program
{
    sqz_decl *decl;
} sqz_program;

#endif
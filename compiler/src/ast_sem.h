#ifndef _AST_SEM_H_
#define _AST_SEM_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"

struct sqz_program;
struct sem_spec_qual;
struct sem_param_decl;
struct sem_designator;
struct sem_declarator;
struct sem_decl_spec;
struct sem_decl;
struct sem_initializer;
struct sem_var_decl;
struct sem_struct_decl;
struct sem_struct_field_decl;
struct sem_func_decl;
struct sem_primary_expr;
struct sem_id;
struct sem_post;
struct sem_expr;
struct sem_args;
struct sem_expr_src_struct_init;
struct sem_initializer_list;
struct sem_designation;
struct sem_expr_src_arr_access;
struct sem_expr_src_func_call;
struct sem_expr_src_member_access;
struct sem_expr_src;
struct sem_cast_expr;
struct sem_init_decl;
struct sem_unary;
struct sem_pre;
struct sem_pre_unary;
struct sem_sizeof;
struct sem_binary_expr;
struct sem_ternary_expr;
struct sem_assign_expr;
struct sem_struct_field;
struct sem_labeled;
struct sem_label;
struct sem_case;
struct sem_default;
struct sem_block_item;
struct sem_compound_stmt;
struct sem_expr_stmt;
struct sem_if;
struct sem_if_else;
struct sem_switch;
struct sem_while;
struct sem_do_while;
struct sem_for;
struct sem_goto;
struct sem_continue;
struct sem_break;
struct sem_return;
struct sem_jump;
struct sem_iter;
struct sem_selection;
struct sem_stmt;

typedef struct symbol
{
    char *name;
    type_t *type;
    struct symbol *prev;
} symbol_t;

struct env
{
    int level;
    struct symbol *symbols;
    struct env *prev;
};

struct _sqz_program;

struct sem_program
{
    struct sem_decl *decl;
};

struct sem_spec_qual
{
    unsigned int qualifier;
    type_t *type;
    struct sem_spec_qual *next;
};

struct sem_param_decl
{
    struct sem_decl_spec *spec;
    struct sem_declarator *decl;
    type_t *type;
};

struct sem_designator
{
    union
    {
        struct sem_id *id;
        struct sem_ternary_expr *expr;
    } val;
    ast_node_type designator_type;
    struct sem_designator *next;
};

struct sem_declarator
{
    type_t *type;
    struct sem_assign_expr *index;
    struct sem_args *args;
    struct sem_id *id;
    struct sem_spec_qual *qual;
    struct sem_declarator *abs_decl;
    struct sem_declarator *next;
};

struct sem_decl_spec
{
    unsigned int storage_class;
    unsigned int qualifier;
};

struct sem_decl
{
    ast_node_type decl_type;
    union
    {
        struct sem_var_decl *var;
        struct sem_func_decl *func;
    } decl;
    struct sem_decl *next;
};

struct sem_initializer
{
    int level;
    struct sem_assign_expr *expr;
    struct sem_initializer_list *init_list;
};

struct sem_var_decl
{
    struct sem_decl_spec *spec;
    type_t *type;
    struct sem_init_decl *decl_list;
    struct sem_var_decl *next;
};

struct sem_struct_decl
{
    struct sem_struct_field_decl *field;
    struct sem_struct_decl *next;
};

struct sem_struct_field_decl
{
    struct sem_decl_spec *spec;
    type_t *type;
    struct sem_struct_field *decl_list;
    struct sem_struct_field_decl *next;
};

struct sem_func_decl
{
    struct sem_decl_spec *spec;
    type_t *return_type;
    struct sem_args *params;
    struct sem_compound_stmt *body;
};

struct sem_primary_expr
{
    ast_node_type primary_type;
    union
    {
        struct sem_id *identifier;
        struct sem_expr *expr;
        int i;
        float f;
        char *s;
    } value;
};

struct sem_id
{
    ast_node_type id_type;
    int scope_level;
    struct sem_decl_spec *spec;
    symrec_t *name;
    type_t *type;
};

struct sem_post
{
    ast_node_type op_type;
    struct sem_expr_src *operand;
};

struct sem_expr
{
    struct sem_assign_expr *expr;
    struct sem_expr *next;
};

struct sem_args
{
    struct sem_param_decl *arg;
    struct sem_args *next;
};

struct sem_expr_src_struct_init
{
    type_t *struct_type;
    struct sem_initializer_list *init_list;
};

struct sem_initializer_list
{
    struct sem_initializer *initializer;
    struct sem_designation *designation;
    struct sem_initializer_list *next;
};

struct sem_designation
{
    struct sem_designator *designator_list;
    struct sem_designation *next;
};

struct sem_expr_src_arr_access
{
    struct sem_expr_src *array;
    struct sem_expr *index;
};

struct sem_expr_src_func_call
{
    struct sem_expr_src *func;
    struct sem_args *args;
};

struct sem_expr_src_member_access
{
    ast_node_type access_type;
    struct sem_expr_src *owner;
    struct sem_id *member_name;
};

struct sem_expr_src
{
    ast_node_type expr_type;
    union
    {
        struct sem_expr_src_func_call *func_call;
        struct sem_expr_src_member_access *member_access;
        struct sem_expr_src_arr_access *arr_access;
        struct sem_expr_src_struct_init *struct_init;
        struct sem_primary_expr *primary_expr;
        struct sem_post *post_inc_dec;
    } expr;
};

struct sem_cast_expr
{
    struct sem_declarator *type;
    ast_node_type cast_type;
    union
    {
        struct sem_cast_expr *cast;
        struct sem_unary *unary;
    } expr;
};

struct sem_init_decl
{
    struct sem_declarator *decl;
    struct sem_initializer *init;
    struct sem_init_decl *next;
};

struct sem_unary
{
    ast_node_type expr_type;
    union
    {
        struct sem_cast_expr *cast;
        struct sem_expr_src *postfix;
        struct sem_sizeof *sizeof_expr;
        struct sem_pre *pre_inc_dec;
        struct sem_pre_unary *pre_unary_op;
        struct sem_declarator *type_name;
    } expr;
};

struct sem_pre
{
    ast_node_type op_type;
    struct sem_unary *operand;
};

struct sem_pre_unary
{
    ast_node_type op_type;
    struct sem_cast_expr *operand;
};

struct sem_sizeof
{
    BOOL is_unary_expr;
    union
    {
        struct sem_unary *unary;
        type_t *type;
    };
};

struct sem_binary_expr
{
    ast_node_type expr_type;
    struct sem_cast_expr *cast_expr;
    struct sem_binary_expr *left;
    union
    {
        struct sem_cast_expr *cast;
        struct sem_binary_expr *binary;
    } right;
};

struct sem_ternary_expr
{
    struct sem_binary_expr *condition;
    struct sem_expr *true_expr;
    struct sem_ternary_expr *false_expr;
    struct sem_binary_expr *binary_expr;
};

struct sem_assign_expr
{
    ast_node_type assign_type;
    struct sem_unary *left;
    struct sem_assign_expr *right;
    struct sem_ternary_expr *ternary_expr;
};

struct sem_labeled
{
    ast_node_type type;
    union
    {
        struct sem_label *label_stmt;
        struct sem_case *case_stmt;
        struct sem_default *default_stmt;
    } stmt;
};

struct sem_label
{
    symrec_t *label;
    struct sem_stmt *stmt;
};

struct sem_case
{
    struct sem_ternary_expr *case_expr;
    struct sem_stmt *stmt;
};

struct sem_default
{
    struct sem_stmt *stmt;
};

struct sem_block_item
{
    ast_node_type decl_or_stmt;
    union
    {
        struct sem_var_decl *decl;
        struct sem_stmt *stmt;
    } item;
    struct sem_block_item *next;
};

struct sem_compound_stmt
{
    struct sem_block_item *block_list;
};

struct sem_expr_stmt
{
    struct sem_expr *expr;
};

struct sem_if
{
    struct sem_expr *expr;
    struct sem_stmt *true_stmt;
};

struct sem_if_else
{
    struct sem_expr *expr;
    struct sem_stmt *true_stmt;
    struct sem_stmt *false_stmt;
};

struct sem_switch
{
    struct sem_expr *expr;
    struct sem_stmt *body;
};

struct sem_while
{
    struct sem_expr *expr;
    struct sem_stmt *body;
};

struct sem_do_while
{
    struct sem_expr *expr;
    struct sem_stmt *body;
};

struct sem_for
{
    struct sem_var_decl *decl;
    struct sem_expr_stmt *cond;
    struct sem_expr *eval;
    struct sem_stmt *body;
};

struct sem_goto
{
    symrec_t *label;
};

struct sem_continue
{
};

struct sem_break
{
};

struct sem_return
{
    struct sem_expr *expr;
};

struct sem_jump
{
    ast_node_type jump_type;
    union
    {
        struct sem_goto *goto_stmt;
        struct sem_continue *continue_stmt;
        struct sem_break *break_stmt;
        struct sem_return *return_stmt;
    } jump;
};

struct sem_iter
{
    ast_node_type iter_type;
    union
    {
        struct sem_while *while_iter;
        struct sem_do_while *do_while_iter;
        struct sem_for *for_iter;
    } iter;
};

struct sem_selection
{
    ast_node_type type;
    union
    {
        struct sem_if *if_selection;
        struct sem_if_else *if_else_selection;
        struct sem_switch *switch_selection;
    } selection;
};

struct sem_stmt
{
    ast_node_type stmt_type;
    union
    {
        struct sem_compound_stmt *compound;
        struct sem_expr_stmt *expr;
        struct sem_labeled *labeled;
        struct sem_iter *iter;
        struct sem_jump *jump;
        struct sem_selection *selection;
    } stmt;
};

struct sem_struct_field
{
    struct sem_decl_spec *spec;
    struct sem_declarator *decl;
    struct sem_ternary_expr *bit_field;
    struct sem_struct_field *next;
};

int sem_program(struct _sqz_program *root, struct sem_program **out);
#endif
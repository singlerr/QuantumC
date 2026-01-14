#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringlib.h"
#include "ast_sem.h"
#include "ast_sqz.h"
#include "diagnostics.h"
#include "ast_typing.h"

struct env *env_list = NULL;
struct env *push_env();
symbol_t *push_symbol(struct env *env, const char *name, type_t *type);
symbol_t *find_symbol(struct env *env, const char *name, BOOL lookup_outer);
void pop_env(struct env *);
identifier *new_identifier(char *name);

void convert_variable_declaration(const sqz_var_decl *var, statement_list **out);
void convert_function_declaration(const sqz_func_decl *func, statement **out);
void convert_assign_expression(const sqz_assign_expr *expr, expression **out);

void convert_program(const struct _sqz_program *p, program **out)
{
    program *prog = IALLOC(program);
    statement *decl_stmt;
    sqz_decl *list = p->decl;

    while (list)
    {
        switch (list->decl_type)
        {
        case AST_VARIABLE_DECLARATION:
            sqz_var_decl *var_decl = list->decl.var;
            convert_variable_declaration(var_decl, &decl_stmt);
            break;
        case AST_FUNCTION_DECLARATION:
            sqz_func_decl *func_decl = list->decl.func;
            convert_function_declaration(func_decl, &decl_stmt);
            break;
        default:
            perror("Unexpected ast");
        }
        list = list->next;
    }

    *out = prog;
}

void convert_variable_declaration(const sqz_var_decl *var, statement_list **out)
{
    sqz_init_decl *decl_list = var->decl_list;
    type_t *type = var->type;
    sqz_declarator *declarator;
    sqz_initializer *initializer;
    statement_list *list = IALLOC(statement_list), *head = list;
    statement *stmt;
    expression *init_expr = NULL;
    while (decl_list)
    {
        declarator = decl_list->decl;
        initializer = decl_list->init;

        stmt = IALLOC(statement);

        if (initializer)
        {
            convert_assign_expression(initializer->expr, &init_expr);
        }
        identifier *id = new_identifier(declarator->id->name->name);
        // qubit declaration
        if (IS_QUBIT(type))
        {
            stmt->classical.qubit_declaration.qubit = id;
        }
        // classical declaration
        else
        {
            if (var->spec->qualifier & QAL_CONST)
            {
                stmt->classical.constant_declaration.identifier = id;

                if (init_expr)
                {
                    stmt->classical.constant_declaration.expression = init_expr;
                    // TODO classical_type
                }
            }
            else
            {
                stmt->classical.declaration.identifier = id;

                if (init_expr)
                {
                    stmt->classical.declaration.init_expression.expr = init_expr;
                    stmt->classical.declaration.init_expression_kind = EXPR_EXPRESSION;
                    // FIXME: How to handle measure?
                }
            }
        }

        if (head->value)
        {
            statement_list *l = IALLOC(statement_list);
            l->value = stmt;
            list_add(statement_list, l, head);
            head = l;
        }
        else
        {
            head->value = stmt;
        }

        decl_list = decl_list->next;
    }

    *out = list;
}

void convert_function_declaration(const sqz_func_decl *func, statement **out)
{
    // func to subroutine definition
    statement *subroutine_def = IALLOC(statement);
    subroutine_def->kind = STMT_DEF;
}

struct env *push_env()
{
    return NULL;
}
symbol_t *push_symbol(struct env *env, const char *name, type_t *type)
{
    return NULL;
}
symbol_t *find_symbol(struct env *env, const char *name, BOOL lookup_outer)
{
    return NULL;
}
void pop_env(struct env *) {}

identifier *new_identifier(char *name)
{
    identifier *id = IALLOC(identifier);
    id->name = strdup(name);
    return id;
}

// type_t *infer_cast_expr(sqz_cast_expr *cast_expr)
// {

//     switch (cast_expr->cast_type)
//     {
//     case AST_EXPR_TYPE_CAST:
//     {
//         type_t *caster_type = cast_expr->type->type;
//         type_t *castee_type = infer_cast_expr(cast_expr->expr.cast);

//         if (!is_casting_compatible(caster_type, castee_type))
//         {
//             LOG_ERROR("Casting type error; Cannot cast from %s to %s", castee_type->name, caster_type->name);
//             return NULL;
//         }

//         return caster_type;
//     }
//     break;
//     default:
//         struct sem_unary *unary = cast_expr->expr.unary;
//         return infer_unary(unary);
//     }

//     return NULL;
// }

// type_t *infer_expr(sqz_expr *expr)
// {
//     return infer_assign_expr(expr->expr);
// }

// type_t *infer_unary(sqz_unary *unary_expr)
// {
//     switch (unary_expr->expr_type)
//     {
//     case AST_EXPR_PRE_INC:
//     {
//         struct _sqz_pre *pre = unary_expr->expr.pre_inc_dec;
//         type_t *t = infer_unary(pre->operand);
//         if (!t || !IS_NUMERIC(t))
//         {
//             LOG_ERROR("Operand type mismatch: pre inc must be numeric", 0);
//             return NULL;
//         }
//         return t;
//     }
//     case AST_EXPR_PRE_DEC:
//     {
//         struct _sqz_pre *pre = unary_expr->expr.pre_inc_dec;
//         type_t *t = infer_unary(pre->operand);
//         if (!t || !IS_NUMERIC(t))
//         {
//             LOG_ERROR("Operand type mismatch: pre dec must be numeric", 0);
//             return NULL;
//         }
//         return t;
//     }
//     case AST_EXPR_UNARY:
//     {
//         sqz_cast_expr *cast = unary_expr->expr.cast;
//         type_t *t = infer_cast_expr(cast);
//         if (!t || !IS_INTEGRAL(t))
//         {
//             LOG_ERROR("Unary operator must be applied to integral types", 0);
//             return NULL;
//         }
//         return t;
//     }
//     case AST_EXPR_SIZEOF:
//         return PRIM_INT->handle;
//     default:
//         return infer_expr_src(unary_expr->expr.postfix);
//     }
// }

// type_t *infer_binary(sqz_binary_expr *binary_expr)
// {
//     if (!binary_expr)
//     {
//         return NULL;
//     }

//     if (binary_expr->cast_expr)
//     {
//         return infer_cast_expr(binary_expr->cast_expr);
//     }

//     type_t *left = infer_binary(binary_expr->left);
//     type_t *right = NULL;

//     switch (binary_expr->expr_type)
//     {
//     case AST_EXPR_MUL:
//     case AST_EXPR_DIV:
//     case AST_EXPR_MOD:
//         right = infer_cast_expr(binary_expr->right.cast);
//         if (!left || !right || !IS_NUMERIC(left) || !IS_NUMERIC(right))
//         {
//             LOG_ERROR("Arithmetic operator requires numeric operands", 0);
//             return NULL;
//         }
//         return left;
//     default:
//         right = infer_binary(binary_expr->right.binary);
//         if (!left || !right)
//         {
//             return NULL;
//         }
//         if (!is_type_compatible(left, right))
//         {
//             LOG_ERROR("Incompatible operand types: %s vs %s", left->name, right->name);
//             return NULL;
//         }
//         return left;
//     }
// }

// type_t *infer_ternary(sqz_ternary_expr *ternary_expr)
// {
//     if (!ternary_expr)
//     {
//         return NULL;
//     }

//     if (ternary_expr->condition)
//     {
//         type_t *cond = infer_binary(ternary_expr->condition);
//         if (!cond || !IS_SCALAR(cond))
//         {
//             LOG_ERROR("Ternary condition must be scalar", 0);
//             return NULL;
//         }

//         type_t *t_true = infer_expr(ternary_expr->true_expr);
//         type_t *t_false = infer_ternary(ternary_expr->false_expr);
//         if (!t_true || !t_false || !is_type_compatible(t_true, t_false))
//         {
//             LOG_ERROR("Type mismatch in ternary branches", 0);
//             return NULL;
//         }
//         return t_true;
//     }

//     return infer_binary(ternary_expr->binary_expr);
// }

// type_t *infer_assign_expr(sqz_assign_expr *assign_expr)
// {
//     if (!assign_expr)
//     {
//         return NULL;
//     }

//     if (assign_expr->left)
//     {
//         type_t *lhs = infer_unary(assign_expr->left);
//         type_t *rhs = infer_assign_expr(assign_expr->right);
//         if (!lhs || !rhs || !is_casting_compatible(lhs, rhs))
//         {
//             LOG_ERROR("Assignment type mismatch", 0);
//             return NULL;
//         }
//         return lhs;
//     }

//     return infer_ternary(assign_expr->ternary_expr);
// }

// int infer_type_size(const type_t *type)
// {
//     if (!type || !type->meta)
//     {
//         return -1;
//     }

//     if (type->meta->size != -1)
//     {
//         return type->meta->size;
//     }

//     typemeta_t *meta = type->meta;
//     int computed = -1;

//     switch (meta->node_type)
//     {
//     case AST_TYPE_STRUCT:
//     {
//         int total = 0;
//         for (struct _sqz_struct_decl *struct_decl = meta->fields; struct_decl; struct_decl = struct_decl->next)
//         {
//             for (struct _sqz_struct_field_decl *field_decl = struct_decl->field; field_decl; field_decl = field_decl->next)
//             {
//                 for (struct _sqz_struct_field *f = field_decl->decl_list; f; f = f->next)
//                 {
//                     type_t *decl_type = (f->decl && f->decl->type) ? f->decl->type : field_decl->type;
//                     int field_size = infer_type_size(decl_type);
//                     if (field_size == -1)
//                     {
//                         LOG_ERROR("Cannot infer type of field: %s", decl_type ? decl_type->name : "<unknown>");
//                         return -1;
//                     }
//                     total += field_size;
//                 }
//             }
//         }
//         computed = total;
//     }
//     break;
//     case AST_TYPE_UNION:
//     {
//         int max_size = 0;
//         for (struct _sqz_struct_decl *struct_decl = meta->fields; struct_decl; struct_decl = struct_decl->next)
//         {
//             for (struct _sqz_struct_field_decl *field_decl = struct_decl->field; field_decl; field_decl = field_decl->next)
//             {
//                 for (struct _sqz_struct_field *f = field_decl->decl_list; f; f = f->next)
//                 {
//                     type_t *decl_type = (f->decl && f->decl->type) ? f->decl->type : field_decl->type;
//                     int field_size = infer_type_size(decl_type);
//                     if (field_size == -1)
//                     {
//                         LOG_ERROR("Cannot infer type of field: %s", decl_type ? decl_type->name : "<unknown>");
//                         return -1;
//                     }
//                     if (field_size > max_size)
//                     {
//                         max_size = field_size;
//                     }
//                 }
//             }
//         }
//         computed = max_size;
//     }
//     break;
//     case AST_TYPE_FUNCTION:
//         computed = 4;
//         break;
//     case AST_TYPE_POINTER:
//         computed = 4;
//         break;
//     default:
//         LOG_ERROR("Unknown type size for: %s", type->name);
//         return -1;
//     }

//     meta->size = computed;
//     return computed;
// }

// static type_t *lookup_member_type(type_t *owner, const char *member_name)
// {
//     if (!owner || !IS_STRUCT(owner) || !owner->next || !owner->next->meta)
//     {
//         return NULL;
//     }

//     typemeta_t *meta = owner->next->meta;
//     for (struct _sqz_struct_decl *sd = meta->fields; sd; sd = sd->next)
//     {
//         for (struct _sqz_struct_field_decl *fd = sd->field; fd; fd = fd->next)
//         {
//             for (struct _sqz_struct_field *f = fd->decl_list; f; f = f->next)
//             {
//                 if (f->decl && f->decl->id && f->decl->id->name && f->decl->id->name->name &&
//                     strcmp(f->decl->id->name->name, member_name) == 0)
//                 {
//                     return f->decl->type ? f->decl->type : fd->type;
//                 }
//             }
//         }
//     }
//     return NULL;
// }

// type_t *infer_expr_src(sqz_expr_src *expr_src)
// {
//     if (!expr_src)
//     {
//         return NULL;
//     }

//     switch (expr_src->expr_type)
//     {
//     case AST_EXPR_ARRAY_ACCESS:
//     {
//         sqz_expr_src_arr_access *arr = expr_src->expr.arr_access;
//         type_t *arr_type = infer_expr_src(arr->array);
//         type_t *idx_type = infer_expr(arr->index);
//         if (!arr_type || !ARRAY_ACCESSIBLE(arr_type) || !idx_type || !IS_INTEGRAL(idx_type))
//         {
//             LOG_ERROR("Invalid array access", 0);
//             return NULL;
//         }
//         return arr_type->next;
//     }
//     case AST_EXPR_FUNCTION_CALL:
//     {
//         sqz_expr_src_func_call *func = expr_src->expr.func_call;
//         type_t *func_type = infer_expr_src(func->func);
//         if (!func_type || !IS_FUNC(func_type) || !func_type->meta || !func_type->meta->func)
//         {
//             LOG_ERROR("Called object is not a function", 0);
//             return NULL;
//         }
//         return func_type->meta->func->return_type;
//     }
//     case AST_EXPR_MEMBER_ACCESS:
//     case AST_EXPR_POINTER_MEMBER_ACCESS:
//     {
//         sqz_expr_src_member_access *mem = expr_src->expr.member_access;
//         type_t *owner = infer_expr_src(mem->owner);
//         if (!owner)
//         {
//             return NULL;
//         }
//         if (mem->access_type == AST_EXPR_POINTER_MEMBER_ACCESS)
//         {
//             if (!IS_PTR(owner) || !owner->next)
//             {
//                 LOG_ERROR("Arrow operator on non-pointer type", 0);
//                 return NULL;
//             }
//             owner = owner->next;
//         }
//         type_t *m = lookup_member_type(owner, mem->member_name->name->name);
//         if (!m)
//         {
//             LOG_ERROR("Unknown struct member: %s", mem->member_name->name->name);
//         }
//         return m;
//     }
//     case AST_EXPR_POST_INC:
//     case AST_EXPR_POST_DEC:
//     {
//         struct _sqz_post *post = expr_src->expr.post_inc_dec;
//         type_t *t = infer_expr_src(post->operand);
//         if (!t || !IS_NUMERIC(t))
//         {
//             LOG_ERROR("Postfix inc/dec requires numeric type", 0);
//             return NULL;
//         }
//         return t;
//     }
//     default:
//     {
//         sqz_primary_expr *p = expr_src->expr.primary_expr;
//         if (!p)
//         {
//             return NULL;
//         }
//         switch (p->primary_type)
//         {
//         case AST_IDENTIFIER:
//             return p->value.identifier ? p->value.identifier->type : NULL;
//         case AST_LITERAL_INTEGER:
//             return PRIM_INT->handle;
//         case AST_LITERAL_FLOAT:
//             return PRIM_FLOAT->handle;
//         case AST_LITERAL_STRING:
//             return PRIM_STRING->handle; // assume char*
//         default:
//             return infer_expr(p->value.expr);
//         }
//     }
//     }
// }
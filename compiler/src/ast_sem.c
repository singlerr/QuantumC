#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringlib.h"
#include "ast_sem.h"
#include "ast_sqz.h"
#include "diagnostics.h"
#include "ast_typing.h"

#ifndef SAFE_FREE
#define SAFE_FREE(ptr) \
    do                 \
    {                  \
        if (ptr)       \
            free(ptr); \
    } while (0)
#endif

#ifndef FREE_LIST
#define FREE_LIST(type, root)   \
    do                          \
    {                           \
        type *r = root;         \
        type *temp;             \
        type *t;                \
        if (r)                  \
        {                       \
            t = r;              \
            while (t)           \
            {                   \
                temp = t->next; \
                free(t);        \
                t = temp;       \
            }                   \
        }                       \
    } while (0)
#endif

struct env *env_list = NULL;
struct env *push_env();
symbol_t *push_symbol(struct env *env, const char *name, type_t *type);
symbol_t *find_symbol(struct env *env, const char *name, BOOL lookup_outer);
void pop_env(struct env *);

int sem_func_decl(struct env *env, sqz_func_decl *src, struct sem_func_decl **out);
int sem_var_decl(struct env *env, sqz_var_decl *var_decl, struct sem_var_decl **out);
int sem_expr(struct env *env, sqz_expr *src, struct sem_expr **out);
int sem_assign_expr(struct env *env, sqz_assign_expr *src, struct sem_assign_expr **out);
int sem_ternary_expr(struct env *env, sqz_ternary_expr *src, struct sem_ternary_expr **out);
int sem_binary_expr(struct env *env, sqz_binary_expr *src, struct sem_binary_expr **out);
int sem_cast_expr(struct env *env, sqz_cast_expr *src, struct sem_cast_expr **out);
int sem_unary(struct env *env, sqz_unary *src, struct sem_unary **out);
int sem_expr_src(struct env *env, sqz_expr_src *src, struct sem_expr_src **out);
int sem_primary_expr(struct env *env, sqz_primary_expr *src, struct sem_primary_expr **out);
int sem_declarator(struct env *env, sqz_declarator *src, struct sem_declarator **out);
int sem_initializer(struct env *env, sqz_initializer *src, struct sem_initializer **out);
int sem_initializer_list(struct env *env, sqz_initializer_list *src, struct sem_initializer_list **out);
int sem_stmt(struct env *env, sqz_stmt *src, struct sem_stmt **out);
int sem_compound_stmt(struct sqz_compound_stmt *src, struct sem_compound_stmt **out);
int sem_block_item(struct env *env, struct _sqz_block_item *src, struct sem_block_item **out);
int sem_decl_spec(sqz_decl_spec *src, struct sem_decl_spec **out);
int sem_args(sqz_args *src, struct sem_args **out);
int sem_param_decl(struct env *env, sqz_param_decl *src, struct sem_param_decl **out);
int sem_selection(struct env *env, struct sqz_selection *src, struct sem_selection **out);
int sem_iter(struct env *env, struct sqz_iter *src, struct sem_iter **out);
int sem_jump(struct env *env, struct sqz_jump *src, struct sem_jump **out);
int sem_labeled(struct env *env, struct sqz_labeled *src, struct sem_labeled **out);

type_t *infer_cast_expr(struct sem_cast_expr *cast_expr);
type_t *infer_expr(struct sem_expr *expr);
type_t *infer_unary(struct sem_unary *unary_expr);
type_t *infer_binary(struct sem_binary_expr *binary_expr);
type_t *infer_ternary(struct sem_ternary_expr *ternary_expr);
type_t *infer_assign_expr(struct sem_assign_expr *assign_expr);
type_t *infer_expr_src(struct sem_expr_src *expr_src);
int infer_type_size(const type_t *type);

struct env *push_env()
{
    struct env *result;
    if (!env_list)
    {
        env_list = IALLOC(struct env);
        env_list->level = 0;
        result = env_list;
    }
    else
    {
        result = IALLOC(struct env);
        result->level = env_list->level + 1;
        result->prev = env_list;
        env_list = result;
    }

    return result;
}

void pop_env(struct env *env)
{
    if (!env_list)
    {
        perror("Dangling environments?");
    }

    if (env_list != env)
    {
        perror("LIFO violation");
    }

    struct env *temp = env->prev;
    env_list = temp;
    symbol_t *sym = env->symbols;

    while (sym)
    {
        symbol_t *s = sym->prev;
        free(sym->name);
        free(sym);
        sym = s;
    }

    free(env);
}

symbol_t *push_symbol(struct env *env, const char *name, type_t *type)
{
    if (!env)
    {
        perror("push env first");
    }

    symbol_t *sym = IALLOC(symbol_t);
    sym->name = strdup(name);
    sym->type = type;

    if (!env->symbols)
    {
        env->symbols = sym;
    }
    else
    {
        sym->prev = env->symbols;
        env->symbols = sym;
    }

    return sym;
}

symbol_t *find_symbol(struct env *cur, const char *name, BOOL lookup_outer)
{
    if (!cur)
    {
        perror("no available environment");
    }

    do
    {
        if (!cur->symbols)
        {
            if (lookup_outer)
            {
                cur = cur->prev;
                continue;
            }
            else
            {
                break;
            }
        }

        symbol_t *sym = cur->symbols;

        while (sym)
        {
            if (strcmp(name, sym->name) == 0)
            {
                return sym;
            }
            sym = sym->prev; // Bug fix: missing iteration
        }

        cur = cur->prev;
    } while (lookup_outer && cur);

    return NULL;
}

int sem_id(sqz_id *src, struct sem_id **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_id *result = IALLOC(struct sem_id);
    result->name = src->name;
    result->id_type = src->id_type;
    result->type = src->type;

    *out = result;
    return VAL_OK;
}

/* sqz_decl_spec to sem_decl_spec */
int sem_decl_spec(sqz_decl_spec *src, struct sem_decl_spec **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_decl_spec *result = IALLOC(struct sem_decl_spec);
    result->storage_class = src->storage_class;
    result->qualifier = src->qualifier;

    *out = result;
    return VAL_OK;
}

int sem_primary_expr(struct env *env, sqz_primary_expr *src, struct sem_primary_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_primary_expr *result = IALLOC(struct sem_primary_expr);
    result->primary_type = src->primary_type;

    switch (src->primary_type)
    {
    case AST_IDENTIFIER:
        symbol_t *s = find_symbol(env, src->value.identifier->name->name, TRUE);
        if (!s)
        {
            LOG_ERROR("Undefined symbol: %s", src->value.identifier->name->name);
            goto fail;
        }

        if (FAILED(sem_id(src->value.identifier, &result->value.identifier)))
        {
            goto fail;
        }
        break;
    case AST_LITERAL_INTEGER:
        result->value.i = src->value.i;
        break;
    case AST_LITERAL_FLOAT:
        result->value.f = src->value.f;
        break;
    case AST_LITERAL_STRING:
        result->value.s = src->value.s;
        break;
    default:
        if (FAILED(sem_expr(env, src->value.expr, &result->value.expr)))
        {
            goto fail;
        }
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_expr_src to sem_expr_src */
int sem_expr_src(struct env *env, sqz_expr_src *src, struct sem_expr_src **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_expr_src *result = IALLOC(struct sem_expr_src);
    result->expr_type = src->expr_type;

    switch (src->expr_type)
    {
    case AST_EXPR_ARRAY_ACCESS:
    {
        struct sem_expr_src_arr_access *arr = IALLOC(struct sem_expr_src_arr_access);
        if (FAILED(sem_expr_src(env, src->expr.arr_access->array, &arr->array)) ||
            FAILED(sem_expr(env, src->expr.arr_access->index, &arr->index)))
        {
            SAFE_FREE(arr);
            goto fail;
        }

        if (arr->array)
        {
            type_t *arr_type = infer_expr_src(arr->array);

            if (!arr_type)
            {
                SAFE_FREE(arr);
                goto fail;
            }

            if (!ARRAY_ACCESSIBLE(arr_type))
            {
                LOG_ERROR("Type mismatch: %s; Only pointer or array type could be accessed via brackets", arr_type->name);
                SAFE_FREE(arr);
                goto fail;
            }
        }

        if (arr->index)
        {
            type_t *index_type = infer_expr(arr->index);

            if (!index_type)
            {
                SAFE_FREE(arr);
                goto fail;
            }

            if (!ARRAY_INDEXIBLE(index_type))
            {
                LOG_ERROR("Type mismatch: %s; Only integral types can be used as index", index_type->name);
                SAFE_FREE(arr);
                goto fail;
            }
        }

        result->expr.arr_access = arr;
    }
    break;
    case AST_EXPR_FUNCTION_CALL:
    {
        struct sem_expr_src_func_call *func = IALLOC(struct sem_expr_src_func_call);
        if (FAILED(sem_expr_src(env, src->expr.func_call->func, &func->func)))
        {
            SAFE_FREE(func);
            goto fail;
        }

        type_t *func_type = infer_expr_src(func->func);
        if (!func_type)
        {
            goto fail;
        }

        if (!IS_FUNC(func_type))
        {
            LOG_ERROR("Type mismatch: %s; Only function types can be called", func_type->name);
            goto fail;
        }

        if (src->expr.func_call->args)
        {
            if (FAILED(sem_args(src->expr.func_call->args, &func->args)))
            {
                SAFE_FREE(func);
                goto fail;
            }
        }
        result->expr.func_call = func;
    }
    break;
    case AST_EXPR_MEMBER_ACCESS:
    case AST_EXPR_POINTER_MEMBER_ACCESS:
    {
        struct sem_expr_src_member_access *member = IALLOC(struct sem_expr_src_member_access);
        member->access_type = src->expr.member_access->access_type;
        if (FAILED(sem_expr_src(env, src->expr.member_access->owner, &member->owner)) ||
            FAILED(sem_id(src->expr.member_access->member_name, &member->member_name)))
        {
            SAFE_FREE(member);
            goto fail;
        }
        result->expr.member_access = member;
    }
    break;
    case AST_EXPR_POST_INC:
    case AST_EXPR_POST_DEC:
    {
        struct sem_post *post = IALLOC(struct sem_post);
        post->op_type = src->expr.post_inc_dec->op_type;
        if (FAILED(sem_expr_src(env, src->expr.post_inc_dec->operand, &post->operand)))
        {
            SAFE_FREE(post);
            goto fail;
        }
        result->expr.post_inc_dec = post;
    }
    break;
    default:
        if (FAILED(sem_primary_expr(env, src->expr.primary_expr, &result->expr.primary_expr)))
        {
            goto fail;
        }
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_cast_expr to sem_cast_expr */
int sem_cast_expr(struct env *env, sqz_cast_expr *src, struct sem_cast_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_cast_expr *result = IALLOC(struct sem_cast_expr);
    result->cast_type = src->cast_type;
    if (src->type)
    {
        if (FAILED(sem_declarator(env, src->type, &result->type)) ||
            FAILED(sem_cast_expr(env, src->expr.cast, &result->expr.cast)))
        {
            goto fail;
        }
    }
    else
    {
        if (FAILED(sem_unary(env, src->expr.unary, &result->expr.unary)))
        {
            goto fail;
        }
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

int sem_unary(struct env *env, sqz_unary *src, struct sem_unary **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_unary *result = IALLOC(struct sem_unary);
    result->expr_type = src->expr_type;

    switch (src->expr_type)
    {
    case AST_UNARY_AMP:
    case AST_UNARY_STAR:
    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_TILDE:
    case AST_UNARY_EXCL:
        if (FAILED(sem_cast_expr(env, src->expr.cast, &result->expr.cast)))
        {
            goto fail;
        }
        break;
    default:
        if (FAILED(sem_expr_src(env, src->expr.postfix, &result->expr.postfix)))
        {
            goto fail;
        }
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_binary_expr to sem_binary_expr */
int sem_binary_expr(struct env *env, sqz_binary_expr *src, struct sem_binary_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_binary_expr *result = IALLOC(struct sem_binary_expr);
    result->expr_type = src->expr_type;

    if (src->cast_expr)
    {
        if (FAILED(sem_cast_expr(env, src->cast_expr, &result->cast_expr)))
        {
            goto fail;
        }
    }
    else
    {
        if (FAILED(sem_binary_expr(env, src->left, &result->left)))
        {
            goto fail;
        }

        switch (src->expr_type)
        {
        case AST_EXPR_MUL:
        case AST_EXPR_DIV:
        case AST_EXPR_MOD:
            if (FAILED(sem_cast_expr(env, src->right.cast, &result->right.cast)))
            {
                goto fail;
            }
            break;
        default:
            if (FAILED(sem_binary_expr(env, src->right.binary, &result->right.binary)))
            {
                goto fail;
            }
            break;
        }
    }

    // binary
    if (result->left)
    {
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_ternary_expr to sem_ternary_expr */
int sem_ternary_expr(struct env *env, sqz_ternary_expr *src, struct sem_ternary_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_ternary_expr *result = IALLOC(struct sem_ternary_expr);

    if (src->condition)
    {
        if (FAILED(sem_binary_expr(env, src->condition, &result->condition)) ||
            FAILED(sem_expr(env, src->true_expr, &result->true_expr)) ||
            FAILED(sem_ternary_expr(env, src->false_expr, &result->false_expr)))
        {
            goto fail;
        }
    }
    else
    {
        if (FAILED(sem_binary_expr(env, src->binary_expr, &result->binary_expr)))
        {
            goto fail;
        }
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_assign_expr to sem_assign_expr */
int sem_assign_expr(struct env *env, sqz_assign_expr *src, struct sem_assign_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_assign_expr *result = IALLOC(struct sem_assign_expr);
    result->assign_type = src->assign_type;

    if (src->left)
    {
        if (FAILED(sem_unary(env, src->left, &result->left)) ||
            FAILED(sem_assign_expr(env, src->right, &result->right)))
        {
            goto fail;
        }
    }
    else
    {
        if (FAILED(sem_ternary_expr(env, src->ternary_expr, &result->ternary_expr)))
        {
            goto fail;
        }
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_expr to sem_expr */
int sem_expr(struct env *env, sqz_expr *src, struct sem_expr **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_expr *root = NULL, *curr, *temp = NULL;
    sqz_expr *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_expr);
        if (FAILED(sem_assign_expr(env, node->expr, &temp->expr)))
        {
            SAFE_FREE(temp);
            goto fail;
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(struct sem_expr, root);
    return VAL_FAILED;
}

/* sqz_type to sem_declarator */
int sem_declarator(struct env *env, sqz_declarator *src, struct sem_declarator **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_declarator *root = NULL, *curr, *temp = NULL;

    sqz_declarator *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_declarator);
        temp->type = node->type;

        if (node->id)
        {
            symbol_t *sym = find_symbol(env, node->id->name->name, FALSE);
            if (sym)
            {
                LOG_ERROR("Duplicate symbol of \"%s\"", node->id->name->name);
                goto fail;
            }
            if (FAILED(sem_id(node->id, &temp->id)))
            {
                goto fail;
            }
        }

        typemeta_t *meta = node->type->meta;

        if (meta->index)
        {
            struct sem_assign_expr *index_expr = NULL;
            if (FAILED(sem_assign_expr(env, meta->index, &index_expr)))
            {
                goto fail;
            }

            type_t *t = infer_assign_expr(index_expr);
            if (!IS_INTEGRAL(t))
            {
                LOG_ERROR("Array size must be integral type", 0);
                goto fail;
            }
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(struct sem_declarator, root);
    return VAL_FAILED;
}

/* sqz_initializer to sem_initializer */
int sem_initializer(struct env *env, sqz_initializer *src, struct sem_initializer **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_initializer *result = IALLOC(struct sem_initializer);
    result->level = src->level;

    if (src->expr)
    {
        if (FAILED(sem_assign_expr(env, src->expr, &result->expr)))
        {
            goto fail;
        }
    }

    if (src->init_list)
    {
        if (FAILED(sem_initializer_list(env, src->init_list, &result->init_list)))
        {
            goto fail;
        }
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_initializer_list to sem_initializer_list */
int sem_initializer_list(struct env *env, sqz_initializer_list *src, struct sem_initializer_list **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_initializer_list *root = NULL, *curr, *temp = NULL;
    sqz_initializer_list *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_initializer_list);

        if (FAILED(sem_initializer(env, node->initializer, &temp->initializer)))
        {
            SAFE_FREE(temp);
            goto fail;
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(struct sem_initializer_list, root);
    return VAL_FAILED;
}

/* sqz_init_decl to sem_init_decl */
int sem_init_decl(struct env *env, sqz_init_decl *src, struct sem_init_decl **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_init_decl *root = NULL, *curr, *temp = NULL;
    sqz_init_decl *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_init_decl);

        if (FAILED(sem_declarator(env, node->decl, &temp->decl)))
        {
            goto fail;
        }

        struct sem_declarator *d = temp->decl;

        while (d)
        {

            if (d->id)
            {
                symbol_t *s = find_symbol(env, d->id->name->name, FALSE);
                if (s)
                {
                    LOG_ERROR("Variable redefinition in the same scope: %s", s->name);
                    goto fail;
                }
                else
                {
                    type_t *t = temp->decl->type;
                    // current var type is func
                    if (d->next && d->next->type && strcmp(d->next->type->name, "func") == 0)
                    {
                        t = d->next->type;
                    }
                    push_symbol(env, d->id->name->name, t);
                }
            }

            d = d->next;
        }

        if (node->init)
        {
            if (FAILED(sem_initializer(env, node->init, &temp->init)))
            {
                goto fail;
            }
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(struct sem_init_decl, root);
    return VAL_FAILED;
}

/* sqz_stmt to sem_stmt */
int sem_stmt(struct env *env, sqz_stmt *src, struct sem_stmt **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_stmt *result = IALLOC(struct sem_stmt);
    result->stmt_type = src->stmt_type;

    switch (src->stmt_type)
    {
    case AST_STMT_COMPOUND:
        if (FAILED(sem_compound_stmt(src->stmt.compound, &result->stmt.compound)))
        {
            goto fail;
        }
        break;
    case AST_STMT_EXPRESSION:
    {
        struct sem_expr_stmt *expr_stmt = IALLOC(struct sem_expr_stmt);
        if (src->stmt.expr && src->stmt.expr->expr)
        {
            if (FAILED(sem_expr(env, src->stmt.expr->expr, &expr_stmt->expr)))
            {
                SAFE_FREE(expr_stmt);
                goto fail;
            }
        }
        result->stmt.expr = expr_stmt;
    }
    break;
    case AST_STMT_LABEL:
    case AST_STMT_CASE:
    case AST_STMT_DEFAULT:
        if (FAILED(sem_labeled(env, src->stmt.labeled, &result->stmt.labeled)))
        {
            goto fail;
        }
        break;
    case AST_STMT_IF:
    case AST_STMT_IF_ELSE:
    case AST_STMT_SWITCH:
        if (FAILED(sem_selection(env, src->stmt.selection, &result->stmt.selection)))
        {
            goto fail;
        }
        break;
    case AST_STMT_WHILE:
    case AST_STMT_DO_WHILE:
    case AST_STMT_FOR:
        if (FAILED(sem_iter(env, src->stmt.iter, &result->stmt.iter)))
        {
            goto fail;
        }
        break;
    case AST_STMT_GOTO:
    case AST_STMT_CONTINUE:
    case AST_STMT_BREAK:
    case AST_STMT_RETURN:
        if (FAILED(sem_jump(env, src->stmt.jump, &result->stmt.jump)))
        {
            goto fail;
        }
        break;
    default:
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_block_item to sem_block_item */
int sem_block_item(struct env *env, struct _sqz_block_item *src, struct sem_block_item **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_block_item *root = NULL, *curr, *temp = NULL;
    struct _sqz_block_item *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_block_item);
        temp->decl_or_stmt = node->decl_or_stmt;

        if (node->decl_or_stmt == AST_VARIABLE_DECLARATION)
        {
            if (FAILED(sem_var_decl(env, node->item.decl, &temp->item.decl)))
            {
                SAFE_FREE(temp);
                goto fail;
            }
        }
        else
        {
            if (FAILED(sem_stmt(env, node->item.stmt, &temp->item.stmt)))
            {
                SAFE_FREE(temp);
                goto fail;
            }
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(struct sem_block_item, root);
    return VAL_FAILED;
}

/* sqz_compound_stmt to sem_compound_stmt */
int sem_compound_stmt(struct sqz_compound_stmt *src, struct sem_compound_stmt **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }
    struct env *cur = push_env();
    struct sem_compound_stmt *result = IALLOC(struct sem_compound_stmt);

    if (FAILED(sem_block_item(cur, src->block_list, &result->block_list)))
    {
        goto fail;
    }

    *out = result;
    pop_env(cur);
    return VAL_OK;
fail:
    SAFE_FREE(result);
    pop_env(cur);
    return VAL_FAILED;
}

/* sqz_func_decl to sem_func_decl */
int sem_func_decl(struct env *env, sqz_func_decl *src, struct sem_func_decl **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_func_decl *result = IALLOC(struct sem_func_decl);
    result->return_type = src->return_type;
    if (src->spec)
    {
        if (FAILED(sem_decl_spec(src->spec, &result->spec)))
        {
            goto fail;
        }
    }

    if (src->params)
    {
        if (FAILED(sem_args(src->params, &result->params)))
        {
            goto fail;
        }
    }

    if (FAILED(sem_compound_stmt(src->body, &result->body)))
    {
        goto fail;
    }

    symbol_t *func_sym = find_symbol(env, (const char *)src->name, TRUE);

    if (func_sym)
    {
        typemeta_t *meta = func_sym->type->meta;
        if (meta->node_type != AST_TYPE_FUNCTION)
        {
            goto fail;
        }

        if (!meta->func)
        {
            goto fail;
        }

        if (meta->node_type == AST_TYPE_FUNCTION)
        {
            if (!is_args_compatible(src->params, meta->args))
            {
                goto fail;
            }

            if (!is_type_compatible(src->return_type, meta->func->return_type))
            {
                goto fail;
            }
        }
    }
    else
    {

        typerec_t *t = NULL;
        for (t = type_table; t; t = t->next)
        {
            if (strcmp(t->name, "func") == 0)
            {
                if (strcmp(t->handle->meta->func->name->name->name, src->name->name->name) == 0)
                {
                    break;
                }
            }
        }

        if (!t)
        {
            goto fail;
        }
        push_symbol(env, src->name->name->name, t->handle);
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

int sem_var_decl(struct env *env, sqz_var_decl *var_decl, struct sem_var_decl **out)
{
    if (!var_decl)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_var_decl *result = IALLOC(struct sem_var_decl);
    result->type = var_decl->type;

    if (var_decl->spec)
    {
        if (FAILED(sem_decl_spec(var_decl->spec, &result->spec)))
        {
            goto fail;
        }
    }

    if (FAILED(sem_init_decl(env, var_decl->decl_list, &result->decl_list)))
    {
        goto fail;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_param_decl to sem_param_decl */
int sem_param_decl(struct env *env, sqz_param_decl *src, struct sem_param_decl **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_param_decl *result = IALLOC(struct sem_param_decl);
    result->type = src->type;

    if (src->spec)
    {
        if (FAILED(sem_decl_spec(src->spec, &result->spec)))
        {
            goto fail;
        }
    }

    if (src->decl)
    {
        if (FAILED(sem_declarator(env, src->decl, &result->decl)))
        {
            goto fail;
        }
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_args to sem_args */
int sem_args(sqz_args *src, struct sem_args **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_args *root = NULL, *curr, *temp = NULL;
    sqz_args *node = src;

    while (node)
    {
        temp = IALLOC(struct sem_args);

        if (node->arg)
        {
            if (FAILED(sem_param_decl(env_list, node->arg, &temp->arg)))
            {
                SAFE_FREE(temp);
                goto fail;
            }
        }

        if (!root)
        {
            root = temp;
            curr = temp;
        }
        else
        {
            curr->next = temp;
            curr = temp;
        }

        node = node->next;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(struct sem_args, root);
    return VAL_FAILED;
}

/* sqz_selection to sem_selection */
int sem_selection(struct env *env, struct sqz_selection *src, struct sem_selection **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_selection *result = IALLOC(struct sem_selection);
    result->type = src->type;

    switch (src->type)
    {
    case AST_STMT_IF:
    {
        struct sem_if *if_sel = IALLOC(struct sem_if);
        if (src->selection.if_selection->expr)
        {
            if (FAILED(sem_expr(env, src->selection.if_selection->expr, &if_sel->expr)))
            {
                SAFE_FREE(if_sel);
                goto fail;
            }
        }
        if (src->selection.if_selection->true_stmt)
        {
            if (FAILED(sem_stmt(env, src->selection.if_selection->true_stmt, &if_sel->true_stmt)))
            {
                SAFE_FREE(if_sel);
                goto fail;
            }
        }
        result->selection.if_selection = if_sel;
    }
    break;
    case AST_STMT_IF_ELSE:
    {
        struct sem_if_else *if_else_sel = IALLOC(struct sem_if_else);
        if (src->selection.if_else_selection->expr)
        {
            if (FAILED(sem_expr(env, src->selection.if_else_selection->expr, &if_else_sel->expr)))
            {
                SAFE_FREE(if_else_sel);
                goto fail;
            }
        }
        if (src->selection.if_else_selection->true_stmt)
        {
            if (FAILED(sem_stmt(env, src->selection.if_else_selection->true_stmt, &if_else_sel->true_stmt)))
            {
                SAFE_FREE(if_else_sel);
                goto fail;
            }
        }
        if (src->selection.if_else_selection->false_stmt)
        {
            if (FAILED(sem_stmt(env, src->selection.if_else_selection->false_stmt, &if_else_sel->false_stmt)))
            {
                SAFE_FREE(if_else_sel);
                goto fail;
            }
        }
        result->selection.if_else_selection = if_else_sel;
    }
    break;
    case AST_STMT_SWITCH:
    {
        struct sem_switch *switch_sel = IALLOC(struct sem_switch);
        if (src->selection.switch_selection->expr)
        {
            if (FAILED(sem_expr(env, src->selection.switch_selection->expr, &switch_sel->expr)))
            {
                SAFE_FREE(switch_sel);
                goto fail;
            }
        }
        if (src->selection.switch_selection->body)
        {
            if (FAILED(sem_stmt(env, src->selection.switch_selection->body, &switch_sel->body)))
            {
                SAFE_FREE(switch_sel);
                goto fail;
            }
        }
        result->selection.switch_selection = switch_sel;
    }
    break;
    default:
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_iter to sem_iter */
int sem_iter(struct env *env, struct sqz_iter *src, struct sem_iter **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_iter *result = IALLOC(struct sem_iter);
    result->iter_type = src->iter_type;

    switch (src->iter_type)
    {
    case AST_STMT_WHILE:
    {
        struct sem_while *while_iter = IALLOC(struct sem_while);
        if (src->iter.while_iter->expr)
        {
            if (FAILED(sem_expr(env, src->iter.while_iter->expr, &while_iter->expr)))
            {
                SAFE_FREE(while_iter);
                goto fail;
            }
        }
        if (src->iter.while_iter->body)
        {
            if (FAILED(sem_stmt(env, src->iter.while_iter->body, &while_iter->body)))
            {
                SAFE_FREE(while_iter);
                goto fail;
            }
        }
        result->iter.while_iter = while_iter;
    }
    break;
    case AST_STMT_DO_WHILE:
    {
        struct sem_do_while *do_while_iter = IALLOC(struct sem_do_while);
        if (src->iter.do_while_iter->expr)
        {
            if (FAILED(sem_expr(env, src->iter.do_while_iter->expr, &do_while_iter->expr)))
            {
                SAFE_FREE(do_while_iter);
                goto fail;
            }
        }
        if (src->iter.do_while_iter->body)
        {
            if (FAILED(sem_stmt(env, src->iter.do_while_iter->body, &do_while_iter->body)))
            {
                SAFE_FREE(do_while_iter);
                goto fail;
            }
        }
        result->iter.do_while_iter = do_while_iter;
    }
    break;
    case AST_STMT_FOR:
    {
        struct sem_for *for_iter = IALLOC(struct sem_for);
        if (src->iter.for_iter->decl)
        {
            if (FAILED(sem_var_decl(env, src->iter.for_iter->decl, &for_iter->decl)))
            {
                SAFE_FREE(for_iter);
                goto fail;
            }
        }
        if (src->iter.for_iter->cond)
        {
            struct sem_expr_stmt *cond = IALLOC(struct sem_expr_stmt);
            if (src->iter.for_iter->cond->expr)
            {
                if (FAILED(sem_expr(env, src->iter.for_iter->cond->expr, &cond->expr)))
                {
                    SAFE_FREE(cond);
                    SAFE_FREE(for_iter);
                    goto fail;
                }
            }
            for_iter->cond = cond;
        }
        if (src->iter.for_iter->eval)
        {
            if (FAILED(sem_expr(env, src->iter.for_iter->eval, &for_iter->eval)))
            {
                SAFE_FREE(for_iter);
                goto fail;
            }
        }
        if (src->iter.for_iter->body)
        {
            if (FAILED(sem_stmt(env, src->iter.for_iter->body, &for_iter->body)))
            {
                SAFE_FREE(for_iter);
                goto fail;
            }
        }
        result->iter.for_iter = for_iter;
    }
    break;
    default:
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_jump to sem_jump */
int sem_jump(struct env *env, struct sqz_jump *src, struct sem_jump **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_jump *result = IALLOC(struct sem_jump);
    result->jump_type = src->jump_type;

    switch (src->jump_type)
    {
    case AST_STMT_GOTO:
    {
        struct sem_goto *goto_stmt = IALLOC(struct sem_goto);
        goto_stmt->label = src->jump.goto_stmt->label;
        result->jump.goto_stmt = goto_stmt;
    }
    break;
    case AST_STMT_CONTINUE:
    {
        struct sem_continue *continue_stmt = IALLOC(struct sem_continue);
        result->jump.continue_stmt = continue_stmt;
    }
    break;
    case AST_STMT_BREAK:
    {
        struct sem_break *break_stmt = IALLOC(struct sem_break);
        result->jump.break_stmt = break_stmt;
    }
    break;
    case AST_STMT_RETURN:
    {
        struct sem_return *return_stmt = IALLOC(struct sem_return);
        if (src->jump.return_stmt->expr)
        {
            if (FAILED(sem_expr(env, src->jump.return_stmt->expr, &return_stmt->expr)))
            {
                SAFE_FREE(return_stmt);
                goto fail;
            }
        }
        result->jump.return_stmt = return_stmt;
    }
    break;
    default:
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

/* sqz_labeled to sem_labeled */
int sem_labeled(struct env *env, struct sqz_labeled *src, struct sem_labeled **out)
{
    if (!src)
    {
        *out = NULL;
        return VAL_OK;
    }

    struct sem_labeled *result = IALLOC(struct sem_labeled);
    result->type = src->type;

    switch (src->type)
    {
    case AST_STMT_LABEL:
    {
        struct sem_label *label_stmt = IALLOC(struct sem_label);
        label_stmt->label = src->stmt.label_stmt->label;
        if (src->stmt.label_stmt->stmt)
        {
            if (FAILED(sem_stmt(env, src->stmt.label_stmt->stmt, &label_stmt->stmt)))
            {
                SAFE_FREE(label_stmt);
                goto fail;
            }
        }
        result->stmt.label_stmt = label_stmt;
    }
    break;
    case AST_STMT_CASE:
    {
        struct sem_case *case_stmt = IALLOC(struct sem_case);
        if (src->stmt.case_stmt->case_expr)
        {
            if (FAILED(sem_ternary_expr(env, src->stmt.case_stmt->case_expr, &case_stmt->case_expr)))
            {
                SAFE_FREE(case_stmt);
                goto fail;
            }
        }
        if (src->stmt.case_stmt->stmt)
        {
            if (FAILED(sem_stmt(env, src->stmt.case_stmt->stmt, &case_stmt->stmt)))
            {
                SAFE_FREE(case_stmt);
                goto fail;
            }
        }
        result->stmt.case_stmt = case_stmt;
    }
    break;
    case AST_STMT_DEFAULT:
    {
        struct sem_default *default_stmt = IALLOC(struct sem_default);
        if (src->stmt.default_stmt->stmt)
        {
            if (FAILED(sem_stmt(env, src->stmt.default_stmt->stmt, &default_stmt->stmt)))
            {
                SAFE_FREE(default_stmt);
                goto fail;
            }
        }
        result->stmt.default_stmt = default_stmt;
    }
    break;
    default:
        break;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

int sem_program(struct _sqz_program *root, struct sem_program **out)
{
    sqz_decl *decl = root->decl;
    struct sem_var_decl *sem_var = NULL;
    struct sem_func_decl *sem_func = NULL;
    struct env *top = push_env();
    while (decl)
    {
        switch (decl->decl_type)
        {
        case AST_VARIABLE_DECLARATION:
            sqz_var_decl *var_decl = decl->decl.var;
            if (FAILED(sem_var_decl(top, var_decl, &sem_var)))
            {
                goto fail;
            }

            break;
        case AST_FUNCTION_DECLARATION:
            sqz_func_decl *func_decl = decl->decl.func;
            if (FAILED(sem_func_decl(top, func_decl, &sem_func)))
            {
                goto fail;
            }

            break;
        default:
            LOG_ERROR("Unknown delcaration type", decl->decl_type);
            break;
        }
        decl = decl->next;
    }

    pop_env(top);

    return VAL_OK;

fail:
    return VAL_FAILED;
}

type_t *infer_cast_expr(struct sem_cast_expr *cast_expr)
{

    switch (cast_expr->cast_type)
    {
    case AST_EXPR_TYPE_CAST:
    {
        type_t *caster_type = cast_expr->type->type;
        type_t *castee_type = infer_cast_expr(cast_expr->expr.cast);

        if (!is_casting_compatible(caster_type, castee_type))
        {
            LOG_ERROR("Casting type error; Cannot cast from %s to %s", castee_type->name, caster_type->name);
            return NULL;
        }

        return caster_type;
    }
    break;
    default:
        struct sem_unary *unary = cast_expr->expr.unary;
        return infer_unary(unary);
    }

    return NULL;
}

type_t *infer_expr(struct sem_expr *expr)
{
    return infer_assign_expr(expr->expr);
}

type_t *infer_unary(struct sem_unary *unary_expr)
{
    switch (unary_expr->expr_type)
    {
    case AST_EXPR_PRE_INC:
    {
        struct sem_pre *pre = unary_expr->expr.pre_inc_dec;
        type_t *t = infer_unary(pre->operand);
        if (!t || !IS_NUMERIC(t))
        {
            LOG_ERROR("Operand type mismatch: pre inc must be numeric", 0);
            return NULL;
        }
        return t;
    }
    case AST_EXPR_PRE_DEC:
    {
        struct sem_pre *pre = unary_expr->expr.pre_inc_dec;
        type_t *t = infer_unary(pre->operand);
        if (!t || !IS_NUMERIC(t))
        {
            LOG_ERROR("Operand type mismatch: pre dec must be numeric", 0);
            return NULL;
        }
        return t;
    }
    case AST_EXPR_UNARY:
    {
        struct sem_cast_expr *cast = unary_expr->expr.cast;
        type_t *t = infer_cast_expr(cast);
        if (!t || !IS_INTEGRAL(t))
        {
            LOG_ERROR("Unary operator must be applied to integral types", 0);
            return NULL;
        }
        return t;
    }
    case AST_EXPR_SIZEOF:
        return PRIM_INT->handle;
    default:
        return infer_expr_src(unary_expr->expr.postfix);
    }
}

type_t *infer_binary(struct sem_binary_expr *binary_expr)
{
    if (!binary_expr)
    {
        return NULL;
    }

    if (binary_expr->cast_expr)
    {
        return infer_cast_expr(binary_expr->cast_expr);
    }

    type_t *left = infer_binary(binary_expr->left);
    type_t *right = NULL;

    switch (binary_expr->expr_type)
    {
    case AST_EXPR_MUL:
    case AST_EXPR_DIV:
    case AST_EXPR_MOD:
        right = infer_cast_expr(binary_expr->right.cast);
        if (!left || !right || !IS_NUMERIC(left) || !IS_NUMERIC(right))
        {
            LOG_ERROR("Arithmetic operator requires numeric operands", 0);
            return NULL;
        }
        return left;
    default:
        right = infer_binary(binary_expr->right.binary);
        if (!left || !right)
        {
            return NULL;
        }
        if (!is_type_compatible(left, right))
        {
            LOG_ERROR("Incompatible operand types: %s vs %s", left->name, right->name);
            return NULL;
        }
        return left;
    }
}

type_t *infer_ternary(struct sem_ternary_expr *ternary_expr)
{
    if (!ternary_expr)
    {
        return NULL;
    }

    if (ternary_expr->condition)
    {
        type_t *cond = infer_binary(ternary_expr->condition);
        if (!cond || !IS_SCALAR(cond))
        {
            LOG_ERROR("Ternary condition must be scalar", 0);
            return NULL;
        }

        type_t *t_true = infer_expr(ternary_expr->true_expr);
        type_t *t_false = infer_ternary(ternary_expr->false_expr);
        if (!t_true || !t_false || !is_type_compatible(t_true, t_false))
        {
            LOG_ERROR("Type mismatch in ternary branches", 0);
            return NULL;
        }
        return t_true;
    }

    return infer_binary(ternary_expr->binary_expr);
}

type_t *infer_assign_expr(struct sem_assign_expr *assign_expr)
{
    if (!assign_expr)
    {
        return NULL;
    }

    if (assign_expr->left)
    {
        type_t *lhs = infer_unary(assign_expr->left);
        type_t *rhs = infer_assign_expr(assign_expr->right);
        if (!lhs || !rhs || !is_casting_compatible(lhs, rhs))
        {
            LOG_ERROR("Assignment type mismatch", 0);
            return NULL;
        }
        return lhs;
    }

    return infer_ternary(assign_expr->ternary_expr);
}

int infer_type_size(const type_t *type)
{
    if (!type || !type->meta)
    {
        return -1;
    }

    if (type->meta->size != -1)
    {
        return type->meta->size;
    }

    typemeta_t *meta = type->meta;
    int computed = -1;

    switch (meta->node_type)
    {
    case AST_TYPE_STRUCT:
    {
        int total = 0;
        for (struct _sqz_struct_decl *struct_decl = meta->fields; struct_decl; struct_decl = struct_decl->next)
        {
            for (struct _sqz_struct_field_decl *field_decl = struct_decl->field; field_decl; field_decl = field_decl->next)
            {
                for (struct _sqz_struct_field *f = field_decl->decl_list; f; f = f->next)
                {
                    type_t *decl_type = (f->decl && f->decl->type) ? f->decl->type : field_decl->type;
                    int field_size = infer_type_size(decl_type);
                    if (field_size == -1)
                    {
                        LOG_ERROR("Cannot infer type of field: %s", decl_type ? decl_type->name : "<unknown>");
                        return -1;
                    }
                    total += field_size;
                }
            }
        }
        computed = total;
    }
    break;
    case AST_TYPE_UNION:
    {
        int max_size = 0;
        for (struct _sqz_struct_decl *struct_decl = meta->fields; struct_decl; struct_decl = struct_decl->next)
        {
            for (struct _sqz_struct_field_decl *field_decl = struct_decl->field; field_decl; field_decl = field_decl->next)
            {
                for (struct _sqz_struct_field *f = field_decl->decl_list; f; f = f->next)
                {
                    type_t *decl_type = (f->decl && f->decl->type) ? f->decl->type : field_decl->type;
                    int field_size = infer_type_size(decl_type);
                    if (field_size == -1)
                    {
                        LOG_ERROR("Cannot infer type of field: %s", decl_type ? decl_type->name : "<unknown>");
                        return -1;
                    }
                    if (field_size > max_size)
                    {
                        max_size = field_size;
                    }
                }
            }
        }
        computed = max_size;
    }
    break;
    case AST_TYPE_FUNCTION:
        computed = 4;
        break;
    case AST_TYPE_POINTER:
        computed = 4;
        break;
    default:
        LOG_ERROR("Unknown type size for: %s", type->name);
        return -1;
    }

    meta->size = computed;
    return computed;
}

static type_t *lookup_member_type(type_t *owner, const char *member_name)
{
    if (!owner || !IS_STRUCT(owner) || !owner->next || !owner->next->meta)
    {
        return NULL;
    }

    typemeta_t *meta = owner->next->meta;
    for (struct _sqz_struct_decl *sd = meta->fields; sd; sd = sd->next)
    {
        for (struct _sqz_struct_field_decl *fd = sd->field; fd; fd = fd->next)
        {
            for (struct _sqz_struct_field *f = fd->decl_list; f; f = f->next)
            {
                if (f->decl && f->decl->id && f->decl->id->name && f->decl->id->name->name &&
                    strcmp(f->decl->id->name->name, member_name) == 0)
                {
                    return f->decl->type ? f->decl->type : fd->type;
                }
            }
        }
    }
    return NULL;
}

type_t *infer_expr_src(struct sem_expr_src *expr_src)
{
    if (!expr_src)
    {
        return NULL;
    }

    switch (expr_src->expr_type)
    {
    case AST_EXPR_ARRAY_ACCESS:
    {
        struct sem_expr_src_arr_access *arr = expr_src->expr.arr_access;
        type_t *arr_type = infer_expr_src(arr->array);
        type_t *idx_type = infer_expr(arr->index);
        if (!arr_type || !ARRAY_ACCESSIBLE(arr_type) || !idx_type || !IS_INTEGRAL(idx_type))
        {
            LOG_ERROR("Invalid array access", 0);
            return NULL;
        }
        return arr_type->next;
    }
    case AST_EXPR_FUNCTION_CALL:
    {
        struct sem_expr_src_func_call *func = expr_src->expr.func_call;
        type_t *func_type = infer_expr_src(func->func);
        if (!func_type || !IS_FUNC(func_type) || !func_type->meta || !func_type->meta->func)
        {
            LOG_ERROR("Called object is not a function", 0);
            return NULL;
        }
        return func_type->meta->func->return_type;
    }
    case AST_EXPR_MEMBER_ACCESS:
    case AST_EXPR_POINTER_MEMBER_ACCESS:
    {
        struct sem_expr_src_member_access *mem = expr_src->expr.member_access;
        type_t *owner = infer_expr_src(mem->owner);
        if (!owner)
        {
            return NULL;
        }
        if (mem->access_type == AST_EXPR_POINTER_MEMBER_ACCESS)
        {
            if (!IS_PTR(owner) || !owner->next)
            {
                LOG_ERROR("Arrow operator on non-pointer type", 0);
                return NULL;
            }
            owner = owner->next;
        }
        type_t *m = lookup_member_type(owner, mem->member_name->name->name);
        if (!m)
        {
            LOG_ERROR("Unknown struct member: %s", mem->member_name->name->name);
        }
        return m;
    }
    case AST_EXPR_POST_INC:
    case AST_EXPR_POST_DEC:
    {
        struct sem_post *post = expr_src->expr.post_inc_dec;
        type_t *t = infer_expr_src(post->operand);
        if (!t || !IS_NUMERIC(t))
        {
            LOG_ERROR("Postfix inc/dec requires numeric type", 0);
            return NULL;
        }
        return t;
    }
    default:
    {
        struct sem_primary_expr *p = expr_src->expr.primary_expr;
        if (!p)
        {
            return NULL;
        }
        switch (p->primary_type)
        {
        case AST_IDENTIFIER:
            return p->value.identifier ? p->value.identifier->type : NULL;
        case AST_LITERAL_INTEGER:
            return PRIM_INT->handle;
        case AST_LITERAL_FLOAT:
            return PRIM_FLOAT->handle;
        case AST_LITERAL_STRING:
            return PRIM_STRING->handle; // assume char*
        default:
            return infer_expr(p->value.expr);
        }
    }
    }
}
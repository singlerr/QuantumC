#include <stdlib.h>

#include "ast_sqz.h"
#include "ast.h"
#include "common.h"
#include "stringlib.h"
#include "diagnostics.h"
#include "symrec.h"

#include <stdlib.h>

#ifndef MK_TYPE
#define MK_TYPE(name, size) mk_type(name, mk_type_meta(size), 0)
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
#ifndef SAFE_FREE
#define SAFE_FREE(ptr) \
    do                 \
    {                  \
        if (ptr)       \
            free(ptr); \
    } while (0)
#endif

int squeeze_program(ast_node *program, sqz_program *out);
int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out);
int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out);
int squeeze_var_init(ast_node *init, sqz_init_decl **out);
int squeeze_decl_spec(ast_node *decl_spec, sqz_decl_spec **out, type_t **type_out);
int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out);
int squeeze_assign_expr(ast_node *assign_expr, sqz_assign_expr **out);
int squeeze_ternary_expr(ast_node *ternary_expr, sqz_ternary_expr **out);
int squeeze_unary_expr(ast_node *unary_expr, sqz_unary **out);
int squeeze_binary_expr(ast_node *binary_expr, sqz_binary_expr **out);
int squeeze_expr(ast_node *expr, sqz_expr **out);
int squeeze_cast_expr(ast_node *cast_expr, sqz_cast_expr **out);
int squeeze_expr_src(ast_node *expr_src, sqz_expr_src **out);
int squeeze_pre(ast_node *pre, struct _sqz_pre **out);
int squeeze_pre_unary(ast_node *pre_unary, struct _sqz_pre_unary **out);
int squeeze_type_name(ast_node *type_name, sqz_declarator **out);
int squeeze_parameter_list(ast_node *args, sqz_args **out);
int squeeze_param_decl(ast_node *param_decl, sqz_param_decl **out);
int squeeze_designator_list(ast_node *designator_list, sqz_designator **out);
int squeeze_initializer(ast_node *initializer, int level, sqz_initializer **out);
int squeeze_id(ast_node *node, ast_identifier_node *id_node, sqz_id **out);
int squeeze_spec_qual(ast_node *node, struct _sqz_spec_qual **out);
int squeeze_abstract_decl(ast_node *abs_decl, sqz_declarator **out);
int squeeze_decl(ast_node *abs_decl, sqz_declarator **out);
int squeeze_compound_stmt(ast_node *comp_stmt, struct sqz_compound_stmt **out);
int squeeze_block_item(ast_node *block_item, struct _sqz_block_item **out);
int squeeze_stmt(ast_node *stmt, sqz_stmt **out);
int squeeze_labeled_stmt(ast_node *stmt, struct sqz_labeled **out);
int squeeze_expr_stmt(ast_node *stmt, struct sqz_expr_stmt **out);
int squeeze_selection_stmt(ast_node *stmt, struct sqz_selection **out);
int squeeze_iter_stmt(ast_node *stmt, struct sqz_iter **out);
int squeeze_jump_stmt(ast_node *stmt, struct sqz_jump **out);
int squeeze_struct_decl(ast_node *decl, sqz_struct_decl **out);
int squeeze_struct_field_decl(ast_node *decl, sqz_struct_field_decl **out);
int squeeze_struct_declarator(ast_node *decl, sqz_struct_field **out);
int squeeze_initializer_list(ast_node *init_list, int level, sqz_initializer_list **out);
int squeeze_designation(ast_node *designation, sqz_designation **out);
int squeeze_enum_decl(ast_node *enum_list, struct _sqz_enum_decl **out);
int squeeze_argument_list(ast_node *args_list, sqz_args **out);

int squeeze_ast(ast_node *root, sqz_program **out)
{
    sqz_program *program = IALLOC(sqz_program);
    int ret = squeeze_program(root, program);
    *out = program;

    return ret;
}

int squeeze_program(ast_node *program, sqz_program *out)
{
    ast_node *translation_unit = program->middle;
    sqz_decl *root = NULL, *curr, *temp = NULL;
    while (translation_unit)
    {
        if (FAILED(squeeze_translation_unit(translation_unit, &temp)))
        {
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

        temp = NULL;
        translation_unit = translation_unit->right;
    }

    out->decl = root;
    return VAL_OK;
fail:
    FREE_LIST(sqz_decl, root);

    return VAL_FAILED;
}

int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out)
{
    ast_node *ext_decl = translation_unit->left;
    sqz_func_decl *func_decl = NULL;
    sqz_var_decl *var_decl = NULL;
    int ret;
    if (!ext_decl)
    {
        return VAL_FAILED;
    }

    switch (ext_decl->node_type)
    {
    case AST_FUNCTION_DECLARATION:
    {
        ret = squeeze_func_declaration(ext_decl, &func_decl);
        if (FAILED(ret))
        {
            ret = VAL_FAILED;
        }
        else
        {
            typemeta_t *meta = mk_type_meta(0);
            meta->node_type = AST_TYPE_FUNCTION;
            meta->func = func_decl;

            type_t *type = mk_type("func", meta, NULL);
            type->name = strdup(func_decl->name->name->name);

            puttype((const char *)type->name, AST_TYPE_FUNCTION, type);
        }
    }
    break;
    case AST_VARIABLE_DECLARATION:
        ret = squeeze_var_declaration(ext_decl, &var_decl);
        if (FAILED(ret))
        {
            ret = VAL_FAILED;
        }
        break;
    default:
        ret = VAL_FAILED;
        break;
    }

    if (!func_decl && !var_decl)
    {
        return ret;
    }

    sqz_decl *result = IALLOC(sqz_decl);
    if (func_decl)
    {
        result->decl_type = AST_FUNCTION_DECLARATION;
        result->decl.func = func_decl;
    }
    else
    {
        result->decl_type = AST_VARIABLE_DECLARATION;
        result->decl.var = var_decl;
    }

    *out = result;

    return ret;
}

int squeeze_decl_spec(ast_node *decl_spec, sqz_decl_spec **out, type_t **type_out)
{
    ast_node *node = decl_spec;
    type_t *root = NULL, *curr, *temp = NULL;
    sqz_struct_decl *struct_decl = NULL;
    struct _sqz_enum_decl *enum_decl = NULL;

    sqz_decl_spec *spec = IALLOC(sqz_decl_spec);

    spec->qualifier = 0;
    spec->storage_class = 0;
    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        // type qualifier / storage class specifier
        if (node->left)
        {
            switch (node->left->node_type)
            {
            case AST_STG_EXTERN:
                spec->storage_class |= STG_EXTERN;
                break;
            case AST_STG_STATIC:
                spec->storage_class |= STG_STATIC;
                break;
            case AST_STG_AUTO:
                spec->storage_class |= STG_AUTO;
                break;
            case AST_STG_REGISTER:
                spec->storage_class |= STG_REGISTER;
                break;
            case AST_QAL_CONST:
                spec->qualifier |= QAL_CONST;
                break;
            case AST_QAL_RESTRICT:
                spec->qualifier |= QAL_RESTRICT;
                break;
            case AST_QAL_VOLATILE:
                spec->qualifier |= QAL_VOLATILE;
                break;
            case AST_STG_TYPEDEF:
                spec->storage_class |= STG_TYPEDEF;
                break;
            default:
                LOG_ERROR("Unknown specifier: %d", node->left->node_type);
                goto fail;
            }
        }
        // type
        else if (node->middle)
        {
            ast_node *t = node->middle;
            if (t->node_type == AST_TYPE_STRUCT_UNION)
            {
                ast_node *companion = t->middle;

                if (!companion)
                {
                    LOG_ERROR("Struct or union structure not found", 0);
                    goto fail;
                }

                ast_identifier_node *id = companion->identifier;
                ast_node *struct_or_union = companion->left;
                ast_node *decl_list = companion->right;

                if (!id || !struct_or_union)
                {
                    LOG_ERROR("Struct or union structure not found", 0);
                    goto fail;
                }

                typemeta_t *meta = mk_type_meta(-1); //

                if (decl_list)
                {
                    if (FAILED(squeeze_struct_decl(decl_list, &struct_decl)))
                    {
                        goto fail;
                    }
                }
                meta->fields = struct_decl;
                meta->node_type = t->node_type;
                type_t *struct_type = mk_type(id->sym->name, meta, NULL);
                temp = mk_type("struct", NULL, struct_type);
            }
            else if (t->node_type == AST_TYPE_ENUM)
            {
                ast_node *companion = t->middle;
                if (!companion || !companion->identifier)
                {
                    LOG_ERROR("Enum structure not found", 0);
                    goto fail;
                }

                ast_node *enum_list = companion->left;
                if (enum_list)
                {
                    if (FAILED(squeeze_enum_decl(enum_list, &enum_decl)))
                    {
                        goto fail;
                    }
                }

                typemeta_t *meta = mk_type_meta(-1);
                meta->node_type = AST_TYPE_ENUM;
                meta->enums = enum_decl;

                type_t *enum_type = mk_type(companion->identifier->sym->name, meta, NULL);
                temp = mk_type("enum", NULL, enum_type);
            }
            else
            {
                if (!t->type)
                {
                    LOG_ERROR("No type provided", 0);
                    goto fail;
                }

                temp = clone_type(t->type->handle);
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
        }

        node = node->right;
    }

    if (type_out)
        *type_out = root;
    if (out)
        *out = spec;
    return VAL_OK;
fail:
    SAFE_FREE(root);
    SAFE_FREE(struct_decl);
    FREE_LIST(struct _sqz_enum_decl, enum_decl);
    FREE_LIST(type_t, root);
    SAFE_FREE(spec);
    return VAL_FAILED;
}

int squeeze_var_init(ast_node *init, sqz_init_decl **out)
{

    if (init->node_type != AST_VARIABLE_DECLARATOR)
    {
        return VAL_FAILED;
    }

    ast_node *declarator = init->middle;
    ast_node *initializer = init->left;
    ast_node *decl = declarator;
    sqz_declarator *direct_decl = NULL;
    sqz_init_decl *result;
    if (FAILED(squeeze_decl(decl, &direct_decl)))
    {
        goto fail;
    }

    result = IALLOC(sqz_init_decl);
    result->decl = direct_decl;
    result->init = NULL;

    if (initializer)
    {
        sqz_initializer *s_init;
        if (FAILED(squeeze_initializer(initializer, 0, &s_init)))
        {
            goto fail;
        }

        result->init = s_init;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(direct_decl);
    return VAL_FAILED;
}

int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out)
{

    ast_node *decl_spec = var_decl->left;
    ast_node *init_list = var_decl->middle;
    sqz_decl_spec *spec = NULL;
    sqz_init_decl *root = NULL, *curr, *temp = NULL;
    sqz_var_decl *var = NULL;
    type_t *type = NULL;
    ast_node *decl_node = NULL;
    if (!decl_spec)
    {
        goto fail;
    }

    if (FAILED(squeeze_decl_spec(decl_spec, &spec, &type)))
    {
        goto fail;
    }

    decl_node = init_list;

    while (decl_node)
    {
        if (decl_node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (!decl_node->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_var_init(decl_node->middle, &temp)))
        {
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

        decl_node = decl_node->right;

        temp = NULL;
    }

    // handle TYPEDEF
    if (spec->storage_class & STG_TYPEDEF)
    {
        if (!root->decl || !root->decl->id)
        {
            LOG_ERROR("typedef expected type name but not found", 0);
            goto fail;
        }

        sqz_id *id = root->decl->id;
        typerec_t *reg_type = gettype(id->name->name);
        if (!reg_type)
        {
            LOG_ERROR("Expected type registered in present but not found: %s", id->name);
            goto fail;
        }

        reg_type->handle = type;
    }

    var = IALLOC(sqz_var_decl);
    var->spec = spec;
    var->decl_list = root;
    var->type = type;
    *out = var;
    return VAL_OK;
fail:
    SAFE_FREE(spec);
    SAFE_FREE(type);
    SAFE_FREE(temp);
    FREE_LIST(sqz_init_decl, root);
    return VAL_FAILED;
}

int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out)
{
    sqz_func_decl *result = NULL;
    sqz_decl_spec *specs = NULL;
    sqz_declarator *decl = NULL;
    type_t *return_type;
    sqz_var_decl *decl_list = NULL;
    sqz_args *func_args = NULL;
    sqz_id *func_id = NULL;
    struct sqz_compound_stmt *body = NULL;

    ast_node *spec_node = func_decl->left;
    ast_node *decl_node = func_decl->middle;
    ast_node *body_node = func_decl->right;
    ast_node *comp_stmt_node;
    ast_node *decl_list_node;
    if (!spec_node || !decl_node || !body_node)
    {
        return VAL_FAILED;
    }

    if (body_node->node_type != AST_FUNCTION_BODY)
    {
        return VAL_FAILED;
    }

    decl_list_node = body_node->left;
    comp_stmt_node = body_node->middle;

    if (FAILED(squeeze_decl_spec(spec_node, &specs, &return_type)))
    {
        goto clean;
    }

    if (FAILED(squeeze_decl(decl_node, &decl)))
    {
        goto clean;
    }

    sqz_declarator *d = decl;

    while (d)
    {
        if (d->type && d->type->meta && d->type->meta->args)
        {
            func_args = d->type->meta->args;
            break;
        }

        d = d->next;
    }

    d = decl;
    while (d)
    {
        if (d->id)
        {
            func_id = d->id;
            break;
        }

        d = d->next;
    }

    if (decl_list_node)
    {
        ast_node *node = decl_list_node;
        sqz_var_decl *root = NULL, *curr, *temp = NULL;
        while (node)
        {
            if (node->node_type != AST_NODE_LIST)
            {
                goto decl_clean;
            }

            ast_node *var_decl_node = node->left;
            if (!var_decl_node)
            {
                goto decl_clean;
            }

            if (FAILED(squeeze_var_declaration(var_decl_node, &temp)))
            {
                goto decl_clean;
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

            node = node->right;
        }

        decl_list = root;
    decl_clean:
        if (temp)
        {
            free(temp);
        }
        FREE_LIST(sqz_var_decl, root);

        return VAL_FAILED;
    }

    if (!comp_stmt_node)
    {
        goto clean;
    }

    if (FAILED(squeeze_compound_stmt(comp_stmt_node, &body)))
    {
        goto clean;
    }

    result = IALLOC(sqz_func_decl);
    result->body = body;
    result->return_type = return_type;
    result->spec = specs;
    result->params = func_args;
    result->name = func_id;
    *out = result;

    return VAL_OK;
clean:
    SAFE_FREE(result);
    SAFE_FREE(specs);
    SAFE_FREE(decl);
    FREE_LIST(sqz_var_decl, decl_list);
    // FIXME: how to safely free all components?
    return VAL_FAILED;
}

int squeeze_assign_expr(ast_node *assign_expr, sqz_assign_expr **out)
{
    int ret;
    sqz_assign_expr *result;
    switch (assign_expr->node_type)
    {
    case AST_EXPR_ASSIGN:
        sqz_unary *left;
        sqz_assign_expr *right;
        if (!assign_expr->left || !assign_expr->middle || !assign_expr->right)
        {
            return VAL_FAILED;
        }

        ret = squeeze_unary_expr(assign_expr->left, &left);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        ret = squeeze_assign_expr(assign_expr->right, &right);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        result = IALLOC(sqz_assign_expr);
        result->assign_type = assign_expr->middle->node_type;
        result->left = left;
        result->right = right;
        break;
    default:
        sqz_ternary_expr *ternary;
        ret = squeeze_ternary_expr(assign_expr, &ternary);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        result = IALLOC(sqz_assign_expr);
        result->left = NULL;
        result->right = NULL;
        result->ternary_expr = ternary;
        break;
    }

    *out = result;

    return VAL_OK;
}

int squeeze_ternary_expr(ast_node *ternary_expr, sqz_ternary_expr **out)
{
    int ret;
    sqz_ternary_expr *result;
    switch (ternary_expr->node_type)
    {
    case AST_EXPR_COND:
        sqz_binary_expr *cond;
        sqz_expr *_true;
        sqz_ternary_expr *_false;
        if (!ternary_expr->left || !ternary_expr->middle || !ternary_expr->right)
        {
            return VAL_FAILED;
        }

        ret = squeeze_binary_expr(ternary_expr->left, &cond);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        ret = squeeze_expr(ternary_expr->middle, &_true);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        ret = squeeze_ternary_expr(ternary_expr->right, &_false);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        result = IALLOC(sqz_ternary_expr);
        result->condition = cond;
        result->true_expr = _true;
        result->false_expr = _false;
        break;
    default:
        sqz_binary_expr *binary_expr;
        ret = squeeze_binary_expr(ternary_expr, &binary_expr);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        result = IALLOC(sqz_ternary_expr);
        result->condition = NULL;
        result->true_expr = NULL;
        result->false_expr = NULL;
        result->binary_expr = binary_expr;
        break;
    }

    *out = result;

    return VAL_OK;
}

int squeeze_expr(ast_node *expr, sqz_expr **out)
{
    sqz_expr *root = NULL, *curr, *temp = NULL;
    ast_node *node = expr;
    sqz_assign_expr *assign_expr = NULL;
    while (node)
    {

        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (!node->left)
        {
            goto fail;
        }

        if (FAILED(squeeze_assign_expr(node->left, &assign_expr)))
        {
            goto fail;
        }

        temp = IALLOC(sqz_expr);
        temp->expr = assign_expr;

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

        node = node->right;

        temp = NULL;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(sqz_expr, root);
    SAFE_FREE(temp);
    SAFE_FREE(assign_expr);
    return VAL_FAILED;
}

int squeeze_unary_expr(ast_node *unary_expr, sqz_unary **out)
{
    sqz_unary *result;
    switch (unary_expr->node_type)
    {
    case AST_EXPR_PRE_INC:
    case AST_EXPR_PRE_DEC:
    case AST_EXPR_SIZEOF:
    {
        struct _sqz_sizeof *s = IALLOC(struct _sqz_sizeof);
        if (unary_expr->left)
        {
            sqz_unary *unary;
            if (FAILED(squeeze_unary_expr(unary_expr, &unary)))
            {
                free(s);
                return VAL_FAILED;
            }
            s->is_unary_expr = TRUE;
            s->expr.unary = unary;
        }
        else if (unary_expr->middle)
        {
            sqz_declarator *type_name;
            if (FAILED(squeeze_type_name(unary_expr->middle, &type_name)))
            {
                free(s);
                return VAL_FAILED;
            }
            s->is_unary_expr = FALSE;
            s->expr.type = type_name->type;
        }

        sqz_unary *expr = IALLOC(sqz_unary);
        expr->expr_type = unary_expr->node_type;
        expr->expr.sizeof_expr = s;
        result = expr;
    }
    break;
    case AST_UNARY_AMP:
    case AST_UNARY_STAR:
    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_TILDE:
    case AST_UNARY_EXCL:
    {
        sqz_cast_expr *cast_expr;
        if (FAILED(squeeze_cast_expr(unary_expr, &cast_expr)))
        {
            return VAL_FAILED;
        }

        sqz_unary *expr = IALLOC(sqz_unary);
        expr->expr_type = unary_expr->node_type;
        expr->expr.cast = cast_expr;
        result = expr;
    }
    break;
    default:
    {
        // handle postfix
        sqz_expr_src *postfix;
        if (FAILED(squeeze_expr_src(unary_expr, &postfix)))
        {
            return VAL_FAILED;
        }

        sqz_unary *expr = IALLOC(sqz_unary);
        expr->expr.postfix = postfix;
        expr->expr_type = unary_expr->node_type;
        result = expr;
    }
    break;
    }

    *out = result;
    return VAL_OK;
}
int squeeze_cast_expr(ast_node *cast_expr, sqz_cast_expr **out)
{
    sqz_cast_expr *result = IALLOC(sqz_cast_expr);
    sqz_unary *unary_expr = NULL;
    sqz_declarator *type_name = NULL;
    sqz_cast_expr *cast = NULL;
    switch (cast_expr->node_type)
    {
    case AST_EXPR_TYPE_CAST:
        if (!cast_expr->left || !cast_expr->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_type_name(cast_expr->left, &type_name)) || FAILED(squeeze_cast_expr(cast_expr->middle, &cast)))
        {
            goto fail;
        }

        result->type = type_name;
        result->expr.cast = cast;
        break;

    default:
        if (FAILED(squeeze_unary_expr(cast_expr, &unary_expr)))
        {
            goto fail;
        }

        result->expr.unary = unary_expr;
        break;
    }

    result->cast_type = cast_expr->node_type;
    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(cast);
    SAFE_FREE(type_name);
    SAFE_FREE(result);
    SAFE_FREE(unary_expr);
    return VAL_FAILED;
}
int squeeze_expr_src(ast_node *expr_src, sqz_expr_src **out)
{
    sqz_expr_src *postfix = IALLOC(sqz_expr_src);
    sqz_primary_expr *primary = NULL;
    switch (expr_src->node_type)
    {
    case AST_EXPR_ARRAY_ACCESS:
    {
        sqz_expr_src *base = NULL;
        sqz_expr *index = NULL;

        if (!expr_src->left || !expr_src->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr_src(expr_src->left, &base)) ||
            FAILED(squeeze_expr(expr_src->middle, &index)))
        {
            goto fail;
        }

        sqz_expr_src_arr_access *arr_access = IALLOC(sqz_expr_src_arr_access);
        arr_access->array = base;
        arr_access->index = index;

        postfix->expr_type = AST_EXPR_ARRAY_ACCESS;
        postfix->expr.arr_access = arr_access;
    }
    break;

    case AST_EXPR_FUNCTION_CALL:
    {
        sqz_expr_src *base = NULL;
        sqz_args *args = NULL;

        if (!expr_src->left)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr_src(expr_src->left, &base)))
        {
            goto fail;
        }

        if (expr_src->middle)
        {
            if (FAILED(squeeze_argument_list(expr_src->middle, &args)))
            {
                goto fail;
            }
        }

        sqz_expr_src_func_call *func_call = IALLOC(sqz_expr_src_func_call);
        func_call->func = base;
        func_call->args = args;

        postfix->expr_type = AST_EXPR_FUNCTION_CALL;
        postfix->expr.func_call = func_call;
    }
    break;

    case AST_EXPR_MEMBER_ACCESS:
    case AST_EXPR_POINTER_MEMBER_ACCESS:
    {
        sqz_expr_src *base = NULL;
        sqz_id *id = NULL;

        if (!expr_src->left || !expr_src->identifier)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr_src(expr_src->left, &base)) ||
            FAILED(squeeze_id(expr_src, expr_src->identifier, &id)))
        {
            goto fail;
        }

        sqz_expr_src_member_access *member_access = IALLOC(sqz_expr_src_member_access);
        member_access->access_type = expr_src->node_type;
        member_access->owner = base;
        member_access->member_name = id;

        postfix->expr_type = expr_src->node_type;
        postfix->expr.member_access = member_access;
    }
    break;

    case AST_EXPR_POST_INC:
    case AST_EXPR_POST_DEC:
    {
        sqz_expr_src *base = NULL;

        if (!expr_src->left)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr_src(expr_src->left, &base)))
        {
            goto fail;
        }

        struct _sqz_post *post = IALLOC(struct _sqz_post);
        post->op_type = expr_src->node_type;
        post->operand = base;

        postfix->expr_type = expr_src->node_type;
        postfix->expr.post_inc_dec = post;
    }
    break;

    case AST_STRUCT_INIT:
    {
        sqz_initializer_list *init = NULL;

        if (!expr_src->left || !expr_src->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_initializer_list(expr_src->middle, 0, &init)))
        {
            goto fail;
        }

        sqz_expr_src_struct_init *struct_init = IALLOC(sqz_expr_src_struct_init);
        struct_init->struct_type = expr_src->left->type ? expr_src->left->type->handle : NULL;

        postfix->expr_type = AST_STRUCT_INIT;
        postfix->expr.struct_init = struct_init;
    }
    break;

    default:
    {
        // Primary expression
        primary = IALLOC(sqz_primary_expr);
        primary->primary_type = expr_src->node_type;

        if (expr_src->node_type == AST_IDENTIFIER)
        {
            if (!expr_src->identifier)
            {
                goto fail;
            }
            sqz_id *id = NULL;
            if (FAILED(squeeze_id(expr_src, expr_src->identifier, &id)))
            {
                goto fail;
            }

            primary->value.identifier = id;
        }
        else if (expr_src->constant)
        {
            switch (expr_src->node_type)
            {
            case AST_LITERAL_INTEGER:
                primary->value.i = expr_src->constant->data.i;
                break;
            case AST_LITERAL_FLOAT:
                primary->value.f = expr_src->constant->data.f;
                break;
            case AST_LITERAL_STRING:
                primary->value.s = expr_src->constant->data.s;
                break;
            default:
                goto fail;
            }
        }
        else
        {
            sqz_expr *paren_expr = NULL;
            if (FAILED(squeeze_expr(expr_src, &paren_expr)))
            {
                SAFE_FREE(primary);
                goto fail;
            }
            primary->value.expr = paren_expr;
        }

        postfix->expr_type = expr_src->node_type;
        postfix->expr.primary_expr = primary;
    }
    break;
    }

    *out = postfix;
    return VAL_OK;

fail:
    SAFE_FREE(postfix);
    SAFE_FREE(primary);
    return VAL_FAILED;
}

int squeeze_pre(ast_node *pre, struct _sqz_pre **out)
{
    struct _sqz_pre *result = IALLOC(struct _sqz_pre);
    sqz_unary *unary = NULL;

    if (!pre || !pre->left)
    {
        goto fail;
    }

    switch (pre->node_type)
    {
    case AST_EXPR_PRE_INC:
    case AST_EXPR_PRE_DEC:
        if (FAILED(squeeze_unary_expr(pre->left, &unary)))
        {
            goto fail;
        }

        result->op_type = pre->node_type;
        result->operand = unary;
        break;

    default:
        goto fail;
    }

    *out = result;
    return VAL_OK;

fail:
    SAFE_FREE(result);
    SAFE_FREE(unary);
    return VAL_FAILED;
}

int squeeze_pre_unary(ast_node *pre_unary, struct _sqz_pre_unary **out)
{
    struct _sqz_pre_unary *result = IALLOC(struct _sqz_pre_unary);
    sqz_cast_expr *cast = NULL;

    if (!pre_unary || !pre_unary->left)
    {
        goto fail;
    }

    switch (pre_unary->node_type)
    {
    case AST_UNARY_AMP:
    case AST_UNARY_STAR:
    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_TILDE:
    case AST_UNARY_EXCL:
        if (FAILED(squeeze_cast_expr(pre_unary->left, &cast)))
        {
            goto fail;
        }

        result->op_type = pre_unary->node_type;
        result->operand = cast;
        break;

    default:
        goto fail;
    }

    *out = result;
    return VAL_OK;

fail:
    SAFE_FREE(result);
    SAFE_FREE(cast);
    return VAL_FAILED;
}

int squeeze_binary_expr(ast_node *binary_expr, sqz_binary_expr **out)
{
    sqz_binary_expr *result;
    sqz_binary_expr *left, *right;
    int ret;
    switch (binary_expr->node_type)
    {
    case AST_EXPR_TYPE_CAST:
    {
        sqz_cast_expr *cast_expr;
        ret = squeeze_cast_expr(binary_expr, &cast_expr);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        sqz_binary_expr *expr = IALLOC(sqz_binary_expr);
        expr->expr_type = AST_EXPR_TYPE_CAST;
        expr->cast_expr = cast_expr;
        result = expr;
    }
    break;
    case AST_EXPR_LOR:
    case AST_EXPR_LAND:
    case AST_EXPR_OR:
    case AST_EXPR_XOR:
    case AST_EXPR_AND:
    case AST_EXPR_EQ:
    case AST_EXPR_NEQ:
    case AST_EXPR_LT:
    case AST_EXPR_GT:
    case AST_EXPR_LEQ:
    case AST_EXPR_GEQ:
    case AST_EXPR_LSHIFT:
    case AST_EXPR_RSHIFT:
    case AST_EXPR_ADD:
    case AST_EXPR_SUB:
    {
        if (!binary_expr->left || !binary_expr->middle)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_binary_expr(binary_expr->left, &left)) || FAILED(squeeze_binary_expr(binary_expr->middle, &right)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = IALLOC(sqz_binary_expr);
        expr->expr_type = binary_expr->node_type;
        expr->left = left;
        expr->right.binary = right;
        result = expr;
    }
    break;
    case AST_EXPR_MUL:
    case AST_EXPR_DIV:
    case AST_EXPR_MOD:
    {
        sqz_cast_expr *cast_expr;
        if (!binary_expr->left || !binary_expr->middle)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_binary_expr(binary_expr->left, &left)) || FAILED(squeeze_cast_expr(binary_expr->middle, &cast_expr)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = IALLOC(sqz_binary_expr);
        expr->expr_type = binary_expr->node_type;
        expr->left = left;
        expr->right.cast = cast_expr;
        result = expr;
    }
    break;
    default:
    {
        // handle cast
        sqz_cast_expr *cast;
        if (FAILED(squeeze_cast_expr(binary_expr, &cast)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = IALLOC(sqz_binary_expr);
        expr->cast_expr = cast;
        expr->expr_type = binary_expr->node_type;
        result = expr;
    }
    break;
    }

    *out = result;
    return VAL_OK;
}

int squeeze_type_name(ast_node *type_name, sqz_declarator **out)
{
    ast_node *specifier_qualifier_list;
    ast_node *abstract_declarator;
    struct _sqz_spec_qual *qualifier = NULL;
    sqz_declarator *abs_decl = NULL;
    sqz_declarator *result;
    if (type_name->node_type != AST_NAME_TYPE)
    {
        return VAL_FAILED;
    }

    specifier_qualifier_list = type_name->left;
    abstract_declarator = type_name->right;

    if (FAILED(squeeze_spec_qual(specifier_qualifier_list, &qualifier)))
    {
        return VAL_FAILED;
    }

    if (abstract_declarator)
    {
        if (FAILED(squeeze_abstract_decl(abstract_declarator, &abs_decl)))
        {
            goto fail;
        }
    }

    result = IALLOC(sqz_declarator);
    result->abs_decl = abs_decl;
    result->qual = qualifier;

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(qualifier);
    SAFE_FREE(abs_decl);
    return VAL_FAILED;
}

int squeeze_parameter_list(ast_node *args, sqz_args **out)
{
    sqz_args *root = NULL, *curr, *temp = NULL;
    ast_node *node = args;

    while (node)
    {
        sqz_param_decl *p_decl;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (!node->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_param_decl(node->middle, &p_decl)))
        {
            goto fail;
        }

        temp = IALLOC(sqz_args);
        temp->arg = p_decl;

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

        temp = NULL;

        node = node->right;
    }

    *out = root;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(sqz_args, root);
    return VAL_FAILED;
}

int squeeze_param_decl(ast_node *param_decl, sqz_param_decl **out)
{
    sqz_decl_spec *spec = NULL;
    sqz_declarator *decl = NULL;
    sqz_declarator *abs_decl = NULL;
    type_t *prim_type = NULL;
    ast_node *spec_node = param_decl->left;
    ast_node *decl_node = param_decl->middle;
    ast_node *abs_decl_node = param_decl->right;

    if (!spec_node)
    {
        return VAL_FAILED;
    }

    if (FAILED(squeeze_decl_spec(spec_node, &spec, &prim_type)))
    {
        goto fail;
    }

    if (decl_node)
    {
        if (FAILED(squeeze_decl(decl_node, &decl)))
        {
            goto fail;
        }
    }

    if (abs_decl_node)
    {
        if (FAILED(squeeze_abstract_decl(abs_decl_node, &abs_decl)))
        {
            goto fail;
        }
    }

    if (decl && abs_decl)
    {
        goto fail;
    }

    sqz_param_decl *result = IALLOC(sqz_param_decl);
    result->spec = spec;
    result->type = prim_type;
    result->decl = decl ? decl : abs_decl;

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(spec);
    SAFE_FREE(decl);
    SAFE_FREE(abs_decl);
    return VAL_FAILED;
}

int squeeze_designator_list(ast_node *designator_list, sqz_designator **out)
{
    sqz_designator *root = NULL, *curr = NULL, *d = NULL;
    ast_node *node;
    if (designator_list->node_type != AST_NODE_LIST)
    {
        return VAL_FAILED;
    }

    node = designator_list;

    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        ast_node *designator_node = node->left;

        switch (designator_node->node_type)
        {
        case AST_ARRAY_ACCESS:
        {
            sqz_ternary_expr *ternary;
            if (!designator_node->left)
            {
                goto fail;
            }

            if (FAILED(squeeze_ternary_expr(designator_node->left, &ternary)))
            {
                goto fail;
            }

            sqz_designator *designator = IALLOC(sqz_designator);
            designator->val.expr = ternary;
            designator->designator_type = AST_ARRAY_ACCESS;

            d = designator;
        }
        break;
        case AST_MEMBER_ACCESS:
        {
            sqz_id *id;
            if (!designator_node->identifier)
            {
                goto fail;
            }

            if (FAILED(squeeze_id(designator_node, designator_node->identifier, &id)))
            {
                goto fail;
            }

            sqz_designator *designator = IALLOC(sqz_designator);
            designator->val.id = id;
            designator->designator_type = AST_MEMBER_ACCESS;

            d = designator;
        }
        break;
        default:
            goto fail;
        }

        if (!root)
        {
            root = d;
            curr = d;
        }
        else
        {
            curr->next = d;
            curr = d;
        }

        node = node->right;
    }

    *out = root;
fail:
    FREE_LIST(sqz_designator, root);
    return VAL_FAILED;
}

int squeeze_id(ast_node *node, ast_identifier_node *id_node, sqz_id **out)
{
    sqz_id *id;

    id = IALLOC(sqz_id);
    id->name = id_node->sym;
    id->id_type = node->node_type;
    id->type = id_node->type;
    id->spec = NULL;

    *out = id;

    return VAL_OK;
}

int squeeze_initializer(ast_node *initializer, int level, sqz_initializer **out)
{
    sqz_initializer_list *init_list = NULL;
    sqz_initializer *i = IALLOC(sqz_initializer);
    // initializer
    switch (initializer->node_type)
    {

        // initializer_list
    case AST_NODE_LIST:
    {

        ast_node *node = initializer;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (FAILED(squeeze_initializer_list(node, level + 1, &init_list)))
        {
            goto fail;
        }

        i->init_list = init_list;
    }
    break;

    default:
    {
        sqz_assign_expr *assign_expr;

        if (initializer->right)
        {
            if (FAILED(squeeze_initializer_list(initializer->right, level + 1, &init_list)))
            {
                goto fail;
            }

            i->init_list = init_list;
        }

        if (FAILED(squeeze_assign_expr(initializer, &assign_expr)))
        {
            goto fail;
        }

        i->init_list = init_list;
        i->expr = assign_expr;
    }
    break;
    }

    *out = i;
    return VAL_OK;
fail:
    if (i)
    {
        free(i);
    }
    return VAL_FAILED;
}

int squeeze_spec_qual(ast_node *node, struct _sqz_spec_qual **out)
{
    struct _sqz_spec_qual *root = NULL, *curr, *temp;

    type_t *t;
    ast_node *cur_node = node;
    ast_node *type_node;
    while (cur_node)
    {
        if (cur_node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }
        // type_specifier
        if (cur_node->left)
        {
            type_node = cur_node->left;
            if (!type_node->type)
            {
                goto fail;
            }

            t = type_node->type->handle;
            temp = IALLOC(struct _sqz_spec_qual);
            temp->type = t;
            temp->qualifier = 0;
        }
        // type_qualifier
        else if (cur_node->middle)
        {
            temp = IALLOC(struct _sqz_spec_qual);
            temp->type = NULL;
            temp->qualifier = 0;
            switch (cur_node->middle->node_type)
            {
            case AST_QAL_CONST:
                temp->qualifier |= QAL_CONST;
                break;
            case AST_QAL_RESTRICT:
                temp->qualifier |= QAL_RESTRICT;
                break;
            case AST_QAL_VOLATILE:
                temp->qualifier |= QAL_VOLATILE;
                break;
            default:
                goto fail;
            }
        }
        else
        {
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

        cur_node = cur_node->right;

        temp = NULL;
    }

    *out = root;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(struct _sqz_spec_qual, root);
    return VAL_FAILED;
}

int squeeze_abstract_decl(ast_node *abs_decl, sqz_declarator **out)
{
    sqz_declarator *root = NULL, *curr = NULL, *t = NULL;
    ast_node *node = abs_decl;

    while (node)
    {

        switch (node->node_type)
        {
        case AST_TYPE_POINTER:
        {
            typemeta_t *meta = mk_type_meta(4);
            meta->node_type = AST_TYPE_POINTER;
            type_t *ptr_type = mk_type("pointer", meta, NULL);
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            s_type->type = ptr_type;
            t = s_type;
        }
        break;
        case AST_TYPE_ARRAY:
        {
            sqz_assign_expr *index;
            ast_node *index_node = node->middle;
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            typemeta_t *meta = mk_type_meta(-1);
            meta->node_type = AST_TYPE_ARRAY;

            if (index_node)
            {
                if (FAILED(squeeze_assign_expr(index_node, &index)))
                {
                    goto fail;
                }
                meta->index = index;
            }
            type_t *arr_type = mk_type("array", meta, NULL);
            arr_type->meta = meta;
            s_type->type = arr_type;
            t = s_type;
        }
        break;
        case AST_TYPE_FUNCTION:
        {
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            sqz_args *args;
            ast_node *args_node = node->left;
            typemeta_t *meta = mk_type_meta(-1);
            meta->node_type = AST_TYPE_FUNCTION;
            if (args_node)
            {
                // handle both parameter_type_list and identifier_list
                if (FAILED(squeeze_parameter_list(args_node, &args)))
                {
                    goto fail;
                }

                meta->args = args;
            }

            type_t *func_type = mk_type("func", meta, NULL);
            func_type->meta = meta;
            s_type->type = func_type;
            t = s_type;
        }
        break;
        default:
            goto fail;
        }

        if (curr && curr->type)
        {
            curr->type->next = t->type;
        }

        if (!root)
        {
            root = t;
            curr = t;
        }
        else
        {
            curr->next = t;
            curr = t;
        }

        node = node->right;
    }
    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(sqz_declarator, root);
    return VAL_FAILED;
}

int squeeze_decl(ast_node *abs_decl, sqz_declarator **out)
{
    sqz_declarator *root = NULL, *curr = NULL, *t = NULL;
    ast_node *node = abs_decl;

    while (node)
    {
        sqz_declarator *t;
        switch (node->node_type)
        {
        case AST_TYPE_POINTER:
        {
            typemeta_t *meta = mk_type_meta(4);
            meta->node_type = AST_TYPE_POINTER;
            type_t *ptr_type = mk_type("pointer", meta, NULL);
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            s_type->type = ptr_type;
            t = s_type;
        }
        break;
        case AST_TYPE_ARRAY:
        {
            sqz_assign_expr *index;
            ast_node *index_node = node->middle;
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            typemeta_t *meta = mk_type_meta(-1);
            meta->node_type = AST_TYPE_ARRAY;

            if (index_node)
            {
                if (FAILED(squeeze_assign_expr(index_node, &index)))
                {
                    goto fail;
                }
                meta->index = index;
            }
            type_t *arr_type = mk_type("array", meta, NULL);
            arr_type->meta = meta;
            s_type->type = arr_type;
            t = s_type;
        }
        break;
        case AST_TYPE_FUNCTION:
        {
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            sqz_args *args;
            ast_node *args_node = node->left;
            typemeta_t *meta = mk_type_meta(-1);
            meta->node_type = AST_TYPE_FUNCTION;
            if (args_node)
            {
                // handle both parameter_type_list and identifier_list
                if (FAILED(squeeze_parameter_list(args_node, &args)))
                {
                    goto fail;
                }

                meta->args = args;
            }

            type_t *func_type = mk_type("func", meta, NULL);
            func_type->meta = meta;
            s_type->type = func_type;
            t = s_type;
        }
        break;
        case AST_IDENTIFIER:
        {
            ast_identifier_node *id_node = node->identifier;
            sqz_id *id;
            if (!id_node)
            {
                return VAL_FAILED;
            }
            sqz_declarator *s_type = IALLOC(sqz_declarator);
            id = IALLOC(sqz_id);
            id->name = id_node->sym;
            id->spec = NULL;
            id->type = id_node->type;
            s_type->id = id;
            t = s_type;
        }
        break;
        default:
            goto fail;
        }

        if (!root)
        {
            root = t;
            curr = t;
        }
        else
        {
            curr->next = t;

            if (curr->type)
            {
                curr->type->next = t->type;
            }

            curr = t;
        }

        node = node->right;
    }

    *out = root;

    return VAL_OK;
fail:
    // Could be replaced by FREE_LIST?
    if (root)
    {
        sqz_declarator *temp;
        t = root;
        while (t)
        {
            temp = t->next;
            free(t);
            t = temp;
        }
    }

    return VAL_FAILED;
}

int squeeze_compound_stmt(ast_node *comp_stmt, struct sqz_compound_stmt **out)
{
    struct sqz_compound_stmt *result;
    struct _sqz_block_item *root = NULL, *curr, *temp = NULL;

    ast_node *node = comp_stmt->middle;
    if (comp_stmt->node_type != AST_STMT_COMPOUND)
    {
        LOG_ERROR("Expected compound statement but found: %u", comp_stmt->node_type);
        return VAL_FAILED;
    }

    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            LOG_ERROR("Expected statement list but found: %u", node->node_type);
            goto fail;
        }

        if (!node->left)
        {
            LOG_ERROR("Corrupted ast", 0);
            goto fail;
        }

        if (FAILED(squeeze_block_item(node->left, &temp)))
        {
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

        node = node->right;

        temp = NULL;
    }

    result = IALLOC(struct sqz_compound_stmt);
    result->block_list = root;

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(struct _sqz_block_item, root);
    return VAL_FAILED;
}
int squeeze_block_item(ast_node *block_item, struct _sqz_block_item **out)
{
    struct _sqz_block_item *result;
    sqz_stmt *stmt = NULL;
    sqz_var_decl *decl = NULL;

    switch (block_item->node_type)
    {
    case AST_VARIABLE_DECLARATION:
        if (FAILED(squeeze_var_declaration(block_item, &decl)))
        {
            return VAL_FAILED;
        }
        break;
    default:
        if (FAILED(squeeze_stmt(block_item, &stmt)))
        {
            return VAL_FAILED;
        }
        break;
    }

    result = IALLOC(struct _sqz_block_item);

    if (stmt)
    {
        result->decl_or_stmt = stmt->stmt_type;
        result->item.stmt = stmt;
    }
    else if (decl)
    {
        result->decl_or_stmt = AST_VARIABLE_DECLARATION;
        result->item.decl = decl;
    }

    *out = result;

    return VAL_OK;
}

int squeeze_stmt(ast_node *stmt, sqz_stmt **out)
{
    sqz_stmt *result = IALLOC(sqz_stmt);
    switch (stmt->node_type)
    {
    case AST_STMT_LABEL:
    case AST_STMT_CASE:
    case AST_STMT_DEFAULT:
        struct sqz_labeled *labeld;
        if (FAILED(squeeze_labeled_stmt(stmt, &labeld)))
        {
            goto fail;
        }

        result->stmt_type = stmt->node_type;
        result->stmt.labeled = labeld;
        break;
    case AST_STMT_COMPOUND:
        struct sqz_compound_stmt *comp_stmt;
        if (FAILED(squeeze_compound_stmt(stmt, &comp_stmt)))
        {
            goto fail;
        }

        result->stmt_type = stmt->node_type;
        result->stmt.compound = comp_stmt;
        break;
    case AST_STMT_EXPRESSION:
        struct sqz_expr_stmt *expr_stmt;
        if (stmt->left)
        {
            if (FAILED(squeeze_expr_stmt(stmt, &expr_stmt)))
            {
                goto fail;
            }
        }
        else
        {
            expr_stmt = IALLOC(struct sqz_expr_stmt);
            expr_stmt->expr = NULL;
        }

        result->stmt.expr = expr_stmt;
        result->stmt_type = AST_STMT_EXPRESSION;
        break;
    case AST_STMT_IF:
    case AST_STMT_IF_ELSE:
    case AST_STMT_SWITCH:
        struct sqz_selection *sel_stmt;
        if (FAILED(squeeze_selection_stmt(stmt, &sel_stmt)))
        {
            goto fail;
        }

        result->stmt.selection = sel_stmt;
        result->stmt_type = stmt->node_type;
        break;
    case AST_STMT_WHILE:
    case AST_STMT_DO_WHILE:
    case AST_STMT_FOR:
        struct sqz_iter *iter_stmt;
        if (FAILED(squeeze_iter_stmt(stmt, &iter_stmt)))
        {
            goto fail;
        }

        result->stmt.iter = iter_stmt;
        result->stmt_type = stmt->node_type;
        break;
    case AST_STMT_GOTO:
    case AST_STMT_CONTINUE:
    case AST_STMT_BREAK:
    case AST_STMT_RETURN:
        struct sqz_jump *jump_stmt;
        if (FAILED(squeeze_jump_stmt(stmt, &jump_stmt)))
        {
            goto fail;
        }

        result->stmt.jump = jump_stmt;
        result->stmt_type = stmt->node_type;
        break;
    default:
        LOG_ERROR("Unknown statement", 0);
        goto fail;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    return VAL_FAILED;
}

int squeeze_labeled_stmt(ast_node *stmt, struct sqz_labeled **out)
{
    struct sqz_labeled *result = IALLOC(struct sqz_labeled);

    sqz_stmt *statement = NULL;
    sqz_ternary_expr *const_expr = NULL;
    switch (stmt->node_type)
    {
    case AST_STMT_LABEL:
    {
        struct sqz_label *label;
        ast_node *stmt_node = stmt->middle;
        ast_identifier_node *id_node = stmt->identifier;
        if (!stmt_node || !id_node)
        {
            goto fail;
        }

        if (FAILED(squeeze_stmt(stmt_node, &statement)))
        {
            goto fail;
        }

        label = IALLOC(struct sqz_label);
        label->label = id_node->sym;
        label->stmt = statement;

        result->type = AST_STMT_LABEL;
        result->stmt.label_stmt = label;
    }
    break;
    case AST_STMT_CASE:
    {
        struct sqz_case *case_stmt;
        ast_node *const_node = stmt->left;
        ast_node *stmt_node = stmt->middle;
        if (!const_node || !stmt_node)
        {
            goto fail;
        }

        if (FAILED(squeeze_ternary_expr(const_node, &const_expr)))
        {
            goto fail;
        }
        if (FAILED(squeeze_stmt(stmt_node, &statement)))
        {
            goto fail;
        }

        case_stmt = IALLOC(struct sqz_case);
        case_stmt->case_expr = const_expr;
        case_stmt->stmt = statement;

        result->type = AST_STMT_CASE;
        result->stmt.case_stmt = case_stmt;
    }
    break;
    case AST_STMT_DEFAULT:
    {
        struct sqz_default *default_stmt;
        ast_node *stmt_node = stmt->middle;
        if (!stmt_node)
        {
            goto fail;
        }

        if (FAILED(squeeze_stmt(stmt_node, &statement)))
        {
            goto fail;
        }

        default_stmt = IALLOC(struct sqz_default);
        default_stmt->stmt = statement;

        result->type = AST_STMT_DEFAULT;
        result->stmt.default_stmt = default_stmt;
    }
    break;
    default:
        goto fail;
    }
    return VAL_OK;
fail:
    SAFE_FREE(result);
    SAFE_FREE(statement);
    SAFE_FREE(const_expr);
    return VAL_FAILED;
}
int squeeze_expr_stmt(ast_node *stmt, struct sqz_expr_stmt **out)
{
    sqz_expr *expr;
    if (stmt->node_type != AST_STMT_EXPRESSION)
    {
        return VAL_FAILED;
    }

    struct sqz_expr_stmt *result = IALLOC(struct sqz_expr_stmt);

    if (stmt->left)
    {
        if (FAILED(squeeze_expr(stmt->left, &expr)))
        {
            // Could use a goto statement to keep the consistency of the code.
            SAFE_FREE(result);
            return VAL_FAILED;
        }

        result->expr = expr;
    }

    *out = result;
    return VAL_OK;
}
int squeeze_selection_stmt(ast_node *stmt, struct sqz_selection **out)
{
    struct sqz_selection *selection;
    sqz_expr *expr = NULL;
    sqz_stmt *true_stmt = NULL, *false_stmt = NULL;
    selection = IALLOC(struct sqz_selection);
    switch (stmt->node_type)
    {
    case AST_STMT_IF:
    {
        if (!stmt->left || !stmt->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr(stmt->left, &expr)) || FAILED(squeeze_stmt(stmt->middle, &true_stmt)))
        {
            goto fail;
        }

        struct sqz_if *if_sel = IALLOC(struct sqz_if);
        if_sel->expr = expr;
        if_sel->true_stmt = true_stmt;

        selection->type = AST_STMT_IF;
        selection->selection.if_selection = if_sel;
    }
    break;
    case AST_STMT_IF_ELSE:
    {
        if (!stmt->left || !stmt->middle || !stmt->right)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr(stmt->left, &expr)) || FAILED(squeeze_stmt(stmt->middle, &true_stmt)) || FAILED(squeeze_stmt(stmt->right, &false_stmt)))
        {
            goto fail;
        }

        struct sqz_if_else *elif_sel = IALLOC(struct sqz_if_else);
        elif_sel->expr = expr;
        elif_sel->false_stmt = false_stmt;
        elif_sel->true_stmt = true_stmt;

        selection->type = AST_STMT_IF_ELSE;
        selection->selection.if_else_selection = elif_sel;
    }
    break;
    case AST_STMT_SWITCH:
    {
        if (!stmt->left || !stmt->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr(stmt->left, &expr)) || FAILED(squeeze_stmt(stmt->middle, &true_stmt)))
        {
            goto fail;
        }

        struct sqz_switch *swtch_sel = IALLOC(struct sqz_switch);
        swtch_sel->expr = expr;
        swtch_sel->body = true_stmt;

        selection->type = AST_STMT_SWITCH;
        selection->selection.switch_selection = swtch_sel;
    }
    break;
    default:
        goto fail;
    }
    *out = selection;
    return VAL_OK;
fail:
    SAFE_FREE(selection);
    SAFE_FREE(expr);
    SAFE_FREE(true_stmt);
    SAFE_FREE(false_stmt);
    return VAL_FAILED;
}
int squeeze_iter_stmt(ast_node *stmt, struct sqz_iter **out)
{
    struct sqz_iter *iter = NULL;
    sqz_expr *expr = NULL;
    sqz_stmt *body = NULL;
    sqz_var_decl *for_init = NULL;
    struct sqz_expr_stmt *for_cond = NULL;
    sqz_expr *for_eval = NULL;

    iter = IALLOC(struct sqz_iter);
    switch (stmt->node_type)
    {
    case AST_STMT_WHILE:
    {
        if (!stmt->left || !stmt->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr(stmt->left, &expr)) || FAILED(squeeze_stmt(stmt->middle, &body)))
        {
            goto fail;
        }

        struct sqz_while *w = IALLOC(struct sqz_while);
        w->body = body;
        w->expr = expr;

        iter->iter_type = AST_STMT_WHILE;
        iter->iter.while_iter = w;
    }
    break;
    case AST_STMT_DO_WHILE:
    {
        if (!stmt->left || !stmt->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_stmt(stmt->left, &body)) || FAILED(squeeze_expr(stmt->middle, &expr)))
        {
            goto fail;
        }

        struct sqz_do_while *dw = IALLOC(struct sqz_do_while);
        dw->expr = expr;
        dw->body = body;

        iter->iter_type = AST_STMT_DO_WHILE;
        iter->iter.do_while_iter = dw;
    }
    break;
    case AST_STMT_FOR:
    {
        ast_node *for_sect = stmt->left;
        if (!for_sect)
        {
            goto fail;
        }
        ast_node *body_stmt = stmt->middle;
        ast_node *expr_1 = for_sect->left, *expr_2 = for_sect->middle, *expr_3 = for_sect->right;

        // second expression and body are mandatory
        if (!expr_2 || !body_stmt)
        {
            goto fail;
        }

        if (FAILED(squeeze_expr_stmt(expr_2, &for_cond)))
        {
            goto fail;
        }

        if (expr_1)
        {
            if (FAILED(squeeze_var_declaration(expr_1, &for_init)))
            {
                goto fail;
            }
        }

        if (expr_3)
        {
            if (FAILED(squeeze_expr(expr_3, &for_eval)))
            {
                goto fail;
            }
        }

        if (FAILED(squeeze_stmt(body_stmt, &body)))
        {
            goto fail;
        }

        struct sqz_for *f = IALLOC(struct sqz_for);
        f->body = body;
        f->decl = for_init;
        f->cond = for_cond;
        f->eval = for_eval;

        iter->iter_type = AST_STMT_FOR;
        iter->iter.for_iter = f;
    }
    break;
    default:
        goto fail;
    }

    *out = iter;
    return VAL_OK;

fail:
    SAFE_FREE(iter);
    SAFE_FREE(expr);
    SAFE_FREE(body);
    SAFE_FREE(for_init);
    SAFE_FREE(for_eval);
    SAFE_FREE(for_cond);

    return VAL_FAILED;
}
int squeeze_jump_stmt(ast_node *stmt, struct sqz_jump **out)
{
    sqz_expr *expr = NULL;
    struct sqz_jump *result = IALLOC(struct sqz_jump);
    switch (stmt->node_type)
    {
    case AST_STMT_GOTO:
        if (!stmt->identifier)
        {
            goto fail;
        }

        struct sqz_goto *g = IALLOC(struct sqz_goto);
        g->label = stmt->identifier->sym;

        result->jump_type = AST_STMT_GOTO;
        result->jump.goto_stmt = g;
        break;
    case AST_STMT_CONTINUE:
        struct sqz_continue *c = IALLOC(struct sqz_continue);
        result->jump_type = AST_STMT_CONTINUE;
        result->jump.continue_stmt = c;
        break;
    case AST_STMT_BREAK:
        struct sqz_break *b = IALLOC(struct sqz_break);
        result->jump_type = AST_STMT_BREAK;
        result->jump.break_stmt = b;
        break;
    case AST_STMT_RETURN:
        sqz_expr *ret = NULL;
        if (stmt->left)
        {
            if (FAILED(squeeze_expr(stmt->left, &ret)))
            {
                goto fail;
            }
        }
        struct sqz_return *r = IALLOC(struct sqz_return);
        r->expr = ret;

        result->jump_type = AST_STMT_RETURN;
        result->jump.return_stmt = r;
        break;
    default:
        goto fail;
    }

    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(result);
    SAFE_FREE(expr);
    return VAL_FAILED;
}

int squeeze_struct_decl(ast_node *decl, sqz_struct_decl **out)
{
    ast_node *node = decl;
    sqz_struct_field_decl *root = NULL, *curr, *temp = NULL;
    sqz_struct_decl *result = NULL;
    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (!node->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_struct_field_decl(node->middle, &temp)))
        {
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
        node = node->right;
    }
    result = IALLOC(sqz_struct_decl);
    result->field = root;
    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(sqz_struct_field_decl, root);
    return VAL_FAILED;
}

int squeeze_struct_field_decl(ast_node *decl, sqz_struct_field_decl **out)
{
    ast_node *decl_spec = decl->left;
    ast_node *init_list = decl->middle;
    sqz_decl_spec *spec = NULL;
    sqz_struct_field *root = NULL, *curr, *temp = NULL;
    sqz_struct_field_decl *var = NULL;
    type_t *type = NULL;
    ast_node *decl_node;
    if (!decl_spec)
    {
        goto fail;
    }

    if (FAILED(squeeze_decl_spec(decl_spec, &spec, &type)))
    {
        goto fail;
    }

    decl_node = init_list;

    while (decl_node)
    {
        if (decl_node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (!decl_node->middle)
        {
            goto fail;
        }

        if (FAILED(squeeze_struct_declarator(decl_node->middle, &temp)))
        {
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

        curr->spec = spec;
        decl_node = decl_node->right;

        temp = NULL;
    }
    var = IALLOC(sqz_struct_field_decl);
    var->spec = spec;
    var->decl_list = root;
    var->type = type;
    *out = var;
    return VAL_OK;
fail:
    SAFE_FREE(spec);
    SAFE_FREE(type);
    SAFE_FREE(temp);
    FREE_LIST(sqz_struct_field, root);
    return VAL_FAILED;
}

int squeeze_struct_declarator(ast_node *decl, sqz_struct_field **out)
{
    sqz_declarator *d = NULL;
    sqz_ternary_expr *bit_field = NULL;

    if (!decl->middle)
    {
        goto fail;
    }

    if (FAILED(squeeze_decl(decl->middle, &d)))
    {
        goto fail;
    }

    // has bit field
    if (decl->right)
    {
        if (FAILED(squeeze_ternary_expr(decl->right, &bit_field)))
        {
            goto fail;
        }
    }

    sqz_struct_field *result = IALLOC(sqz_struct_field);
    result->bit_field = bit_field;
    result->decl = d;
    *out = result;
    return VAL_OK;
fail:
    SAFE_FREE(decl);
    SAFE_FREE(bit_field);
    return VAL_FAILED;
}

int squeeze_initializer_list(ast_node *init_list, int level, sqz_initializer_list **out)
{
    sqz_initializer_list *root = NULL, *curr, *temp = NULL;
    sqz_initializer *initializer = NULL;
    sqz_designation *designation = NULL;
    ast_node *node = init_list;

    while (node)
    {
        initializer = NULL;
        designation = NULL;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        ast_node *desg_node = node->left;
        ast_node *init_node = node->middle;

        if (!init_node)
        {
            goto fail;
        }

        if (FAILED(squeeze_initializer(init_node, level, &initializer)))
        {
            goto fail;
        }

        initializer->level += level;

        if (!desg_node)
        {
            if (FAILED(squeeze_designation(desg_node, &designation)))
            {
                goto fail;
            }
        }

        temp = IALLOC(sqz_initializer_list);
        temp->initializer = initializer;
        temp->designation = designation;

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

        node = node->right;
    }

    *out = root;

    return VAL_OK;
fail:
    SAFE_FREE(designation);
    SAFE_FREE(initializer);
    FREE_LIST(sqz_initializer_list, root);
    return VAL_FAILED;
}

int squeeze_designation(ast_node *designation, sqz_designation **out)
{
    sqz_designation *root = NULL, *curr, *temp = NULL;
    ast_node *node = designation;
    sqz_designator *desginator = NULL;

    while (node)
    {
        desginator = NULL;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (FAILED(squeeze_designator_list(node->left, &desginator)))
        {
            goto fail;
        }

        temp = IALLOC(sqz_designation);
        temp->designator_list = desginator;

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

        node = node->right;
    }

    *out = root;
    return VAL_OK;
fail:
    FREE_LIST(sqz_designation, root);
    return VAL_FAILED;
}

int squeeze_enum_decl(ast_node *enum_list, struct _sqz_enum_decl **out)
{
    struct _sqz_enum_decl *root = NULL, *curr = NULL, *temp = NULL;
    ast_node *node = enum_list;

    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        ast_node *enumerator = node->middle;
        if (!enumerator || enumerator->node_type != AST_ENUM || !enumerator->identifier)
        {
            goto fail;
        }

        sqz_id *id = NULL;
        if (FAILED(squeeze_id(enumerator, enumerator->identifier, &id)))
        {
            goto fail;
        }

        sqz_assign_expr *value = NULL;
        if (enumerator->middle)
        {
            if (FAILED(squeeze_assign_expr(enumerator->middle, &value)))
            {
                goto fail;
            }
        }

        temp = IALLOC(struct _sqz_enum_decl);
        temp->id = id;
        temp->value = value;

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

        node = node->right;
        temp = NULL;
    }

    *out = root;
    return VAL_OK;
fail:
    SAFE_FREE(temp);
    FREE_LIST(struct _sqz_enum_decl, root);
    return VAL_FAILED;
}

int squeeze_argument_list(ast_node *args_list, sqz_args **out)
{
    ast_node *node = args_list;
    ast_node *expr_node;
    sqz_assign_expr *expr;
    sqz_args *root = NULL, *curr = NULL, *temp = NULL;
    while (node)
    {
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        expr_node = node->middle;

        if (FAILED(squeeze_assign_expr(expr_node, &expr)))
        {
            goto fail;
        }

        temp = IALLOC(sqz_args);
        temp->expr = expr;

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

        node = node->right;
    }

    *out = root;

    return VAL_OK;
fail:
    FREE_LIST(sqz_args, root);
    SAFE_FREE(temp);
    return VAL_FAILED;
}
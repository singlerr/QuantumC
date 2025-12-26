#include "ast_sqz.h"
#include "ast.h"
#include "common.h"
#include "stringlib.h"
#include <stdlib.h>

#define MK_TYPE(name, size) mk_type(name, mk_type_meta(size), 0)

int squeeze_program(ast_node *program, sqz_program *out);
int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out);
int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out);
int squeeze_var_init(ast_node *init, sqz_var_decl **out);
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
int squeeze_type_name(ast_node *type_name, sqz_type **out);
int squeeze_parameter_list(ast_node *args, sqz_args **out);

int squeeze_ast(ast_node *root, sqz_program **out)
{
    sqz_program *program = (sqz_program *)malloc(sizeof(sqz_program));
    int ret = squeeze_program(root, program);
    *out = program;

    return ret;
}

int squeeze_program(ast_node *program, sqz_program *out)
{
    ast_node *translation_unit = program->middle;
    sqz_decl *decl;
    while (translation_unit)
    {
        if (FAILED(squeeze_translation_unit(translation_unit, &decl)))
        {
            return VAL_FAILED;
        }

        translation_unit = translation_unit->right;
    }

    return VAL_OK;
}

int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out)
{
    ast_node *ext_decl = translation_unit->left;
    sqz_func_decl *func_decl;
    sqz_var_decl *var_decl;
    int ret;
    if (!ext_decl)
    {
        return VAL_FAILED;
    }

    switch (ext_decl->node_type)
    {
    case AST_FUNCTION_DECLARATION:
        ret = squeeze_func_declaration(ext_decl, &func_decl);
        if (FAILED(ret))
        {
            free(func_decl);
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

    return ret;
}

int squeeze_decl_spec(ast_node *decl_spec, sqz_decl_spec **out, type_t **type_out)
{
    ast_node *s = decl_spec->left;
    ast_node *t = decl_spec->middle;
    typerec_t *type, *root_type = NULL;

    sqz_decl_spec *spec = ALLOC(sqz_decl_spec);

    spec->qualifier = 0;
    spec->storage_class = 0;
    while (s)
    {
        switch (s->node_type)
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
        default:
            free(spec);
            return VAL_FAILED;
        }
        s = s->left;
    }

    if (!t)
    {
        free(spec);
        return VAL_FAILED;
    }

    while (t)
    {
        if (t->node_type != AST_TYPE_SPECIFIER)
        {
            free(spec);
            if (root_type)
            {
                if (type != root_type)
                {
                    free(root_type);
                }
                free(type);
            }
            return VAL_FAILED;
        }

        ast_node *type_node = t->left;

        if (!type_node->type)
        {
            free(spec);
            if (root_type)
            {
                if (type != root_type)
                {
                    free(root_type);
                }
                free(type);
            }
            return VAL_FAILED;
        }

        type_t *tt = clone_type(type_node->type->handle);

        if (!root_type)
        {
            root_type = tt;
        }
        else
        {
            type->next = tt;
        }
        type = tt;
        t = t->right;
    }
    *type_out = root_type;
    *out = spec;
    return VAL_OK;
}

int squeeze_var_init(ast_node *init, sqz_var_decl **out)
{
    ast_node *decl_node = init->middle;
    if (!decl_node || decl_node->node_type != AST_VARIABLE_DECLARATOR)
    {
        return VAL_FAILED;
    }

    ast_node *declarator = decl_node->middle;
    ast_node *initializer = decl_node->left;

    ast_node *decl = declarator;
    sqz_type *root = NULL, *type;
    while (decl)
    {
        sqz_type *t;
        switch (decl->node_type)
        {
        case AST_TYPE_POINTER:
            type_t *ptr_type = MK_TYPE("pointer", 4);
            sqz_type *s_type = ALLOC(sqz_type);
            s_type->type = ptr_type;
            s_type->index = NULL;
            s_type->args = NULL;
            t = s_type;
            break;
        case AST_TYPE_ARRAY:
            sqz_assign_expr *index;
            ast_node *index_node = decl->middle;
            sqz_type *s_type;
            if (!index_node)
            {
                s_type = ALLOC(sqz_type);
                s_type->index = NULL;
                s_type->args = NULL;
                s_type->type = NULL;
            }
            else
            {
                if (FAILED(squeeze_assign_expr(index_node, &index)))
                {
                    return VAL_FAILED;
                }
                s_type = ALLOC(sqz_type);
                s_type->index = index;
                s_type->args = NULL;
                s_type->type = NULL;
            }
            t = s_type;
            break;
        case AST_TYPE_FUNCTION:
            sqz_type *s_type;
            sqz_args *args;
            ast_node *args_node = decl->left;

            if (!args_node)
            {
                s_type = ALLOC(sqz_type);
                s_type->args = NULL;
                s_type->index = NULL;
                s_type->type = NULL;
            }
            else
            {
                // handle both parameter_type_list and identifier_list
                if (FAILED(squeeze_parameter_list(args_node, &args)))
                {
                    return VAL_FAILED;
                }
                s_type = ALLOC(sqz_type);
                s_type->args = args;
                s_type->index = NULL;
                s_type->type = NULL;
            }

            t = s_type;
            break;
        case AST_IDENTIFIER:
            ast_identifier_node *id_node = decl->identifier;
            sqz_id *id;
            if (!id_node)
            {
                return VAL_FAILED;
            }
            s_type = ALLOC(sqz_type);
            s_type->args = NULL;
            s_type->index = NULL;

            id = ALLOC(sqz_id);
            id->name = strdup(id_node->sym->name);
            id->spec = NULL;
            id->type = id_node->type;

            s_type->id = id;
            t = s_type;
            break;
        default:
            sqz_type *tt = root;
            sqz_type *tt2;
            while (tt)
            {
                tt2 = tt->next;
                free(tt);
                tt = tt2;
            }
            return VAL_FAILED;
        }

        if (!root)
        {
            root = t;
        }
        else
        {
            type->next = t;
        }

        type = t;
        decl = decl->right;
    }

    return VAL_OK;
}

int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out)
{

    ast_node *decl_spec = var_decl->left;
    ast_node *init = var_decl->middle;
    sqz_decl_spec *spec;
    sqz_var_decl *init_decl;
    sqz_var_decl *var = NULL;
    type_t *type;
    if (!decl_spec)
    {
        return VAL_FAILED;
    }

    if (FAILED(squeeze_decl_spec(decl_spec, &spec, &type)))
    {
        return VAL_FAILED;
    }

    if (init)
    {
        if (init->node_type != AST_NODE_LIST)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_var_init(init, &init_decl)))
        {
            return VAL_FAILED;
        }

        var = init_decl;
        init_decl = init_decl->next;

        init = init->right;

        while (init)
        {
            if (init->node_type != AST_NODE_LIST)
            {
                return VAL_FAILED;
            }

            if (FAILED(squeeze_var_init(init, &init_decl)))
            {
                init_decl->next = NULL;
                return VAL_FAILED;
            }
            init_decl->type = type;
            init_decl->spec = spec;
            init = init->right;
            init_decl = init_decl->next;
        }
    }
    var->type = type;
    *out = var;
    return VAL_OK;
}
int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out)
{
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
        if (!assign_expr->left || !assign_expr->middle || assign_expr->right)
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

        result = ALLOC(sqz_assign_expr);
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

        result = ALLOC(sqz_assign_expr);
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

        result = ALLOC(sqz_ternary_expr);
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

        result = ALLOC(sqz_ternary_expr);
        result->condition = NULL;
        result->true_expr = NULL;
        result->false_expr = NULL;
        result->binary_expr = binary_expr;
        break;
    }

    *out = result;

    return VAL_FAILED;
}

int squeeze_expr(ast_node *expr, sqz_expr **out)
{
}

int squeeze_unary_expr(ast_node *unary_expr, sqz_unary **out)
{
    sqz_unary *result;
    switch (unary_expr->node_type)
    {
    case AST_EXPR_PRE_INC:
    case AST_EXPR_PRE_DEC:
    case AST_EXPR_SIZEOF:
        sqz_unary *unary;
        if (FAILED(squeeze_unary_expr(unary_expr, &unary)))
        {
            return VAL_FAILED;
        }

        sqz_unary *expr = ALLOC(sqz_unary);
        expr->expr_type = unary_expr->node_type;
        break;
    case AST_UNARY_AMP:
    case AST_UNARY_STAR:
    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_TILDE:
    case AST_UNARY_EXCL:
        sqz_cast_expr *cast_expr;
        if (FAILED(squeeze_cast_expr(unary_expr, &cast_expr)))
        {
            return VAL_FAILED;
        }

        sqz_unary *expr = ALLOC(sqz_unary);
        expr->expr_type = unary_expr->node_type;
        expr->expr.cast = cast_expr;
        result = expr;
        break;
    default:
        // handle postfix
        sqz_expr_src *postfix;
        if (FAILED(squeeze_expr_src(unary_expr, &postfix)))
        {
            return VAL_FAILED;
        }

        sqz_unary *expr = ALLOC(sqz_unary);
        expr->expr.postfix = postfix;
        expr->expr_type = unary_expr->node_type;
        result = expr;
        break;
    }

    *out = result;
    return VAL_OK;
}
int squeeze_cast_expr(ast_node *cast_expr, sqz_cast_expr **out)
{
}
int squeeze_expr_src(ast_node *expr_src, sqz_expr_src **out)
{
}
int squeeze_pre(ast_node *pre, struct _sqz_pre **out)
{
}
int squeeze_pre_unary(ast_node *pre_unary, struct _sqz_pre_unary **out)
{
}

int squeeze_binary_expr(ast_node *binary_expr, sqz_binary_expr **out)
{
    sqz_binary_expr *result;
    sqz_binary_expr *left, *right;
    int ret;
    switch (binary_expr->node_type)
    {
    case AST_EXPR_TYPE_CAST:
        sqz_cast_expr *cast_expr;
        ret = squeeze_cast_expr(binary_expr, &cast_expr);
        if (FAILED(ret))
        {
            return VAL_FAILED;
        }

        sqz_binary_expr *expr = ALLOC(sqz_binary_expr);
        expr->expr_type = AST_EXPR_TYPE_CAST;
        expr->cast_expr = cast_expr;
        result = expr;
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
        if (!binary_expr->left || !binary_expr->middle)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_binary_expr(binary_expr->left, &left)) || FAILED(squeeze_binary_expr(binary_expr->middle, &right)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = ALLOC(sqz_binary_expr);
        expr->expr_type = binary_expr->node_type;
        expr->left = left;
        expr->right.binary = right;
        result = expr;
        break;
    case AST_EXPR_MUL:
    case AST_EXPR_DIV:
    case AST_EXPR_MOD:
        sqz_cast_expr *cast_expr;
        if (!binary_expr->left || !binary_expr->middle)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_binary_expr(binary_expr->left, &left)) || FAILED(squeeze_cast_expr(binary_expr->middle, &cast_expr)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = ALLOC(sqz_binary_expr);
        expr->expr_type = binary_expr->node_type;
        expr->left = left;
        expr->right.cast = cast_expr;
        result = expr;
        break;
    default:
        // handle cast
        sqz_cast_expr *cast;
        if (FAILED(squeeze_cast_expr(binary_expr, &cast)))
        {
            return VAL_FAILED;
        }
        sqz_binary_expr *expr = ALLOC(sqz_binary_expr);
        expr->cast_expr = cast;
        expr->expr_type = binary_expr->node_type;
        result = expr;
        break;
    }

    *out = result;
    return VAL_OK;
}

int squeeze_type_name(ast_node *type_name, sqz_type **out)
{
}

int squeeze_parameter_list(ast_node *args, sqz_args **out)
{
}
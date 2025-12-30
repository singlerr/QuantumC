#include "ast_sqz.h"
#include "ast.h"
#include "common.h"
#include "stringlib.h"
#include <stdlib.h>

#define MK_TYPE(name, size) mk_type(name, mk_type_meta(size), 0)
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
#define SAFE_FREE(ptr) \
    do                 \
    {                  \
        if (ptr)       \
            free(ptr); \
    } while (0)

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
int squeeze_type_name(ast_node *type_name, sqz_type **out);
int squeeze_parameter_list(ast_node *args, sqz_args **out);
int squeeze_designator_list(ast_node *designator_list, sqz_designator **out);
int squeeze_initializer(ast_node *initializer, sqz_initializer *parent, sqz_initializer **out);
int squeeze_id(ast_node *node, ast_identifier_node *id_node, sqz_id **out);
int squeeze_spec_qual(ast_node *node, struct _sqz_spec_qual **out);
int squeeze_abstract_decl(ast_node *abs_decl, sqz_type **out);
int squeeze_decl(ast_node *abs_decl, sqz_type **out);
int squeeze_compound_stmt(ast_node *comp_stmt, struct sqz_compound_stmt **out);
int squeeze_block_item(ast_node *block_item, struct _sqz_block_item **out);
int squeeze_stmt(ast_node *stmt, sqz_stmt **out);
int squeeze_labeled_stmt(ast_node *stmt, struct sqz_labeled **out);
int squeeze_expr_stmt(ast_node *stmt, struct sqz_expr_stmt **out);
int squeeze_selection_stmt(ast_node *stmt, struct sqz_selection **out);
int squeeze_iter_stmt(ast_node *stmt, struct sqz_iter **out);
int squeeze_jump_stmt(ast_node *stmt, struct sqz_jump **out);

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
    type_t *type, *root_type = NULL;

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
            type->link = tt;
        }
        type = tt;
        t = t->right;
    }
    if (type_out)
        *type_out = root_type;

    if (out)
        *out = spec;
    return VAL_OK;
}

int squeeze_var_init(ast_node *init, sqz_init_decl **out)
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
    sqz_init_decl *result;
    sqz_type *s_type;
    while (decl)
    {
        sqz_type *t;
        switch (decl->node_type)
        {
        case AST_TYPE_POINTER:
            type_t *ptr_type = MK_TYPE("pointer", 4);
            s_type = ALLOC(sqz_type);
            s_type->type = ptr_type;
            s_type->index = NULL;
            s_type->args = NULL;
            t = s_type;
            break;
        case AST_TYPE_ARRAY:
            sqz_assign_expr *index;
            ast_node *index_node = decl->middle;
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
            id->name = id_node->sym;
            id->spec = NULL;
            id->type = id_node->type;

            s_type->id = id;
            t = s_type;
            break;
        default:
            goto fail;
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

    result = ALLOC(sqz_init_decl);
    result->decl = root;
    result->init = NULL;

    if (initializer)
    {
        sqz_initializer *s_init;
        if (FAILED(squeeze_initializer(initializer, NULL, &s_init)))
        {
            goto fail;
        }

        result->init = s_init;
    }

    *out = result;
    return VAL_OK;
fail:
    sqz_type *tt = root;
    sqz_type *tt2;
    while (tt)
    {
        tt2 = tt->next;
        free(tt);
        tt = tt2;
    }

    if (result)
    {
        free(result);
    }
    return VAL_FAILED;
}

int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out)
{

    ast_node *decl_spec = var_decl->left;
    ast_node *init_list = var_decl->middle;
    sqz_decl_spec *spec;
    sqz_init_decl *root = NULL, *curr, *temp;
    sqz_var_decl *var = NULL;
    type_t *type;
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
        if (decl_node->type != AST_NODE_LIST)
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
    }
    var->spec = spec;
    var->decl_list = root;
    var->type = type;
    *out = var;
fail:
    SAFE_FREE(spec);
    SAFE_FREE(type);
    SAFE_FREE(temp);
    FREE_LIST(sqz_init_decl, root);
    return VAL_OK;
}
int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out)
{
    sqz_func_decl *result = NULL;
    sqz_decl_spec *specs = NULL;
    sqz_type *decl = NULL;
    type_t *return_type;
    sqz_var_decl *decl_list = NULL;
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

    result = ALLOC(sqz_func_decl);
    result->body = body;
    result->return_type = return_type;
    result->spec = specs;
    result->params = NULL;

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
    ast_node *node = expr;
    sqz_expr *root = NULL, *curr, *temp;
    while (node)
    {
        sqz_assign_expr *assign_expr;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        if (FAILED(squeeze_assign_expr(node, &assign_expr)))
        {
            goto fail;
        }

        if (!root)
        {
            root = assign_expr;
            curr = assign_expr;
        }
        else
        {
            curr->next = assign_expr;
            curr = assign_expr;
        }

        node = node->right;
    }

    *out = root;

    return VAL_OK;

fail:
    if (root)
    {
        sqz_expr *e = root;

        while (e)
        {
            temp = e->next;
            free(e);
            e = temp;
        }
    }

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
    sqz_cast_expr *result;
    switch (cast_expr->node_type)
    {
    case AST_EXPR_TYPE_CAST:
        sqz_type *type_name;
        sqz_cast_expr *expr;

        if (!cast_expr->left || !cast_expr->middle)
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_type_name(cast_expr->left, &type_name)))
        {
            return VAL_FAILED;
        }

        if (FAILED(squeeze_cast_expr(cast_expr->middle, &expr)))
        {
            free(type_name);
            return VAL_FAILED;
        }

        sqz_cast_expr *c = ALLOC(sqz_cast_expr);
        c->type = type_name;
        c->cast = expr;

        result = c;
        break;
    default:
        sqz_unary *unary_expr;
        if (FAILED(squeeze_unary_expr(cast_expr, &unary_expr)))
        {
            return VAL_FAILED;
        }

        sqz_cast_expr *expr = ALLOC(sqz_cast_expr);
        expr->cast = NULL;
        expr->type = NULL;
        expr->unary = unary_expr;

        result = expr;
        break;
    }

    *out = result;
    return VAL_OK;
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
    ast_node *specifier_qualifier_list;
    ast_node *abstract_declarator;
    struct _sqz_spec_qual *qualifier;
    sqz_type *abs_decl;
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

    if (FAILED(squeeze_abstract_decl(abstract_declarator, &abs_decl)))
    {
        free(qualifier);
        return VAL_FAILED;
    }

    return VAL_OK;
}

int squeeze_parameter_list(ast_node *args, sqz_args **out)
{
    sqz_args *root = NULL, *curr, *temp;
    ast_node *node = args;

    while (node)
    {
        ast_node *param_decl_node;
        ast_node *decl_spec_node, *decl_node, *abs_decl_node;
        sqz_decl_spec *spec;
        sqz_type *decl = NULL, *abs_decl = NULL;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }
        param_decl_node = node->middle;

        if (!param_decl_node)
        {
            goto fail;
        }

        decl_spec_node = param_decl_node->left;
        decl_node = param_decl_node->middle;
        abs_decl_node = param_decl_node->right;

        if (!decl_spec_node)
        {
            goto fail;
        }

        if (FAILED(squeeze_decl_spec(decl_spec_node, &spec, NULL)))
        {
            goto fail;
        }

        if (decl_node && !abs_decl_node)
        {
            if (FAILED(squeeze_decl(decl_node, &decl)))
            {
                goto fail;
            }
        }
        else if (!decl_node && abs_decl_node)
        {
            if (FAILED(squeeze_abstract_decl(abs_decl_node, &abs_decl)))
            {
                goto fail;
            }
        }
        else
        {
            goto fail;
        }

        sqz_args *result = ALLOC(sqz_args);
        sqz_param_decl *param_decl = ALLOC(sqz_param_decl);
        param_decl->decl = decl ? decl : abs_decl;
        param_decl->spec = spec;

        result->arg = param_decl;

        if (!root)
        {
            root = result;
            curr = result;
        }
        else
        {
            curr->next = result;
            curr = result;
        }

        node = node->right;
    }

    *out = root;

    return VAL_OK;
fail:
    if (root)
    {
        sqz_args *a = root;
        while (a)
        {
            temp = a->next;
            free(a);
            a = temp;
        }
    }

    return VAL_FAILED;
}

int squeeze_designator_list(ast_node *designator_list, sqz_designator **out)
{
    sqz_designator *root = NULL, *curr, *d, *temp;
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
            sqz_ternary_expr *ternary;
            if (!designator_node->left)
            {
                goto fail;
            }

            if (FAILED(squeeze_ternary_expr(designator_node->left, &ternary)))
            {
                goto fail;
            }

            sqz_designator *designator = ALLOC(sqz_designator);
            designator->val.expr = ternary;
            designator->designator_type = AST_ARRAY_ACCESS;

            d = designator;
            break;
        case AST_MEMBER_ACCESS:
            sqz_id *id;
            if (!designator_node->identifier)
            {
                goto fail;
            }

            if (FAILED(squeeze_id(designator_node, designator_node->identifier, &id)))
            {
                goto fail;
            }

            sqz_designator *designator = ALLOC(sqz_designator);
            designator->val.id = id;
            designator->designator_type = AST_MEMBER_ACCESS;

            d = designator;
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

    return VAL_OK;
fail:
    if (root)
    {
        d = root;
        while (d)
        {
            temp = d->next;
            free(d);
            d = temp;
        }
    }

    return VAL_FAILED;
}

int squeeze_id(ast_node *node, ast_identifier_node *id_node, sqz_id **out)
{
    sqz_id *id;

    id = ALLOC(sqz_id);
    id->name = id_node->sym;
    id->id_type = node->node_type;
    id->type = id_node->type;
    id->spec = NULL;

    *out = id;

    return VAL_OK;
}

int squeeze_initializer(ast_node *initializer, sqz_initializer *parent, sqz_initializer **out)
{
    sqz_initializer *init;
    sqz_initializer *i = ALLOC(sqz_initializer);
    // initializer
    switch (initializer->node_type)
    {
        // initializer_list
    case AST_NODE_LIST:
        ast_node *node = initializer;
        ast_node *designator_node;
        ast_node *initializer_node;

        sqz_designator *designator = NULL;
        if (node->node_type != AST_NODE_LIST)
        {
            goto fail;
        }

        designator_node = node->left;
        initializer_node = node->middle;

        if (!initializer_node)
        {
            goto fail;
        }

        if (parent)
        {
            i->level = parent->level + 1;
        }
        else
        {
            i->level = 0;
        }

        if (FAILED(squeeze_initializer(initializer_node, i, &init)))
        {
            goto fail;
        }

        if (designator_node)
        {
            if (FAILED(squeeze_designator_list(designator_node, &designator)))
            {
                goto fail;
            }

            i->designator = designator;
        }

        i->next = init;
        break;

    default:
        sqz_assign_expr *assign_expr;
        sqz_initializer *init;

        if (initializer->right)
        {
            if (FAILED(squeeze_initializer(initializer->right, i, &init)))
            {
                goto fail;
            }

            i->next = init;
        }

        if (FAILED(squeeze_assign_expr(initializer, &assign_expr)))
        {
            goto fail;
        }

        if (parent)
        {
            i->level = parent->level + 1;
        }
        else
        {
            i->level = 0;
        }

        i->expr = assign_expr;
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
    ast_node *type_node, *qual_node;
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

            t = type_node->type;
            temp = ALLOC(struct _sqz_spec_qual);
            temp->type = t;
            temp->qualifier = NULL;
        }
        // type_qualifier
        else if (cur_node->middle)
        {
            temp = ALLOC(struct _sqz_spec_qual);
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
    }
    return VAL_OK;
fail:

    if (temp)
    {
        free(temp);
    }
    if (root)
    {
        struct _sqz_spec_qual *q;
        temp = root;
        while (temp)
        {
            q = temp->next;
            free(temp);
            temp = q;
        }
    }

    return VAL_FAILED;
}

int squeeze_abstract_decl(ast_node *abs_decl, sqz_type **out)
{
    sqz_type *root = NULL, *curr, *t;
    ast_node *node = abs_decl;

    while (node)
    {
        sqz_type *t;
        switch (node->node_type)
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
            ast_node *index_node = node->middle;
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
            ast_node *args_node = node->left;

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
            curr = t;
        }

        node = node->right;
    }

    return VAL_OK;
fail:

    if (root)
    {
        sqz_type *temp;
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

int squeeze_decl(ast_node *abs_decl, sqz_type **out)
{
    sqz_type *root = NULL, *curr, *t;
    ast_node *node = abs_decl;

    while (node)
    {
        sqz_type *t;
        switch (node->node_type)
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
            ast_node *index_node = node->middle;
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
            ast_node *args_node = node->left;

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
            ast_identifier_node *id_node = node->identifier;
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
            curr = t;
        }

        node = node->right;
    }

    return VAL_OK;
fail:

    if (root)
    {
        sqz_type *temp;
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
        return VAL_FAILED;
    }

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
    }

    result = ALLOC(struct sqz_compound_stmt);
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

    result = ALLOC(struct _sqz_block_item);

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
    sqz_stmt *result = ALLOC(sqz_stmt);
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
            expr_stmt = ALLOC(struct sqz_expr_stmt);
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
    case AST_STMT_FOR_EXPR:
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
        goto fail;
    }

    return VAL_OK;
fail:
    free(result);
    return VAL_FAILED;
}

int squeeze_labeled_stmt(ast_node *stmt, struct sqz_labeled **out)
{
    struct sqz_labeled *result = ALLOC(struct sqz_labeled);

    sqz_stmt *statement = NULL;
    sqz_expr *const_expr = NULL;
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

        label = ALLOC(struct sqz_label);
        label->label = id_node->sym;
        label->stmt = statement;
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

        if (FAILED(squeeze_expr(const_node, &const_expr)))
        {
            goto fail;
        }
        if (FAILED(squeeze_stmt(stmt_node, &statement)))
        {
            goto fail;
        }
    }
    break;
    case AST_STMT_DEFAULT:
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
int squeeze_expr_stmt(ast_node *stmt, struct sqz_expr_stmt **out) {}
int squeeze_selection_stmt(ast_node *stmt, struct sqz_selection **out) {}
int squeeze_iter_stmt(ast_node *stmt, struct sqz_iter **out) {}
int squeeze_jump_stmt(ast_node *stmt, struct sqz_jump **out) {}
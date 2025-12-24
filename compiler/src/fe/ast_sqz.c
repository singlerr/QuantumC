#include "ast_sqz.h"
#include "ast.h"
#include "common.h"
#include <stdlib.h>

#define MK_TYPE(name, size) mk_type(name, mk_type_meta(size), 0)

int squeeze_program(ast_node *program, sqz_program *out);
int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out);
int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out);
int squeeze_var_init(ast_node *init, sqz_var_decl **out);
int squeeze_decl_spec(ast_node *decl_spec, sqz_decl_spec **out, type_t **type_out);
int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out);

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

    sqz_decl_spec *spec = (sqz_decl_spec *)malloc(sizeof(sqz_decl_spec));

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
    type_t *root = NULL, *type;
    while (decl)
    {
        type_t *t;
        switch (decl->node_type)
        {
        case AST_TYPE_POINTER:
            type_t *ptr_type = MK_TYPE("pointer", 4);
            t = ptr_type;
            break;
        case AST_TYPE_ARRAY:
            ast_node *index_node = decl->middle;
            break;
        case AST_TYPE_FUNCTION:
            break;
        case AST_IDENTIFIER:
            break;
        default:
            break;
        }

        if (!root)
        {
            root = t;
        }
        else
        {
            type->link = t;
        }

        type = t;
        decl = decl->right;
    }
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
#include "ast_sqz.h"
#include "ast.h"
#include "common.h"
#include <stdlib.h>

int squeeze_program(ast_node *program, sqz_program *out);
int squeeze_translation_unit(ast_node *translation_unit, sqz_decl **out);
int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out);
int squeeze_var_init(ast_node *init, sqz_var_decl **out);

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
            free(decl);
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
            free(var_decl);
        }
        break;
    default:
        ret = VAL_FAILED;
        break;
    }

    return ret;
}
int squeeze_var_init(ast_node *init, sqz_var_decl **out)
{
}

int squeeze_var_declaration(ast_node *var_decl, sqz_var_decl **out)
{

    ast_node *decl_spec = var_decl->left;
    ast_node *init = var_decl->middle;

    if (!decl_spec)
    {
        return VAL_FAILED;
    }

    sqz_var_decl *var = (sqz_var_decl *)malloc(sizeof(sqz_var_decl));

    if (!init)
    {
    }
}
int squeeze_func_declaration(ast_node *func_decl, sqz_func_decl **out)
{
}
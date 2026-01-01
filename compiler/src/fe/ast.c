#include "ast.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stringlib.h"
#include "common.h"
int scope_level = 0;

ast_node *new_ast_node(ast_node_type node_type, const ast_identifier_node *id_node, const ast_const_node *const_node, const typerec_t *type, const ast_node *left, const ast_node *middle, const ast_node *right)
{
    ast_node *node = IALLOC(ast_node);
    node->node_type = node_type;
    node->identifier = (ast_identifier_node *)id_node;
    node->left = (ast_node *)left;
    node->right = (ast_node *)right;
    node->middle = (ast_node *)middle;
    node->type = (typerec_t *)type;
    node->constant = (ast_const_node *)const_node;
    return node;
}

int register_type_if_required(ast_node *decl, ast_node *identifier)
{
    ast_node *node = decl;
    BOOL has_typedef = FALSE;
    while (node)
    {
        if (node->node_type == AST_STG_TYPEDEF)
        {
            has_typedef = TRUE;
            break;
        }
        node = node->left;
    }

    if (!has_typedef)
    {
        return 0;
    }

    identifier = identifier->middle;

    if (!identifier || identifier->node_type != AST_VARIABLE_DECLARATOR)
    {
        perror("typedef requires its type name");
    }

    node = decl;
    while (node->right)
    {
        if (node->node_type == AST_TYPE_SPECIFIER)
            break;
        node = node->right;
    }

    int size = 0;
    typerec_t *t = NULL;
    while (node->node_type == AST_TYPE_SPECIFIER)
    {
        ast_node *type_node = node->left;
        if (!type_node->type)
        {
            perror("unknown type");
        }

        typerec_t *sub = clone_type_rec(type_node->type);
        if (!t)
        {
            t = sub;
        }
        else
        {
            t->next = sub;
        }

        size = sub->handle->meta->size;

        node = type_node->left;
    }

    PUT_TYPE(identifier->middle->identifier->sym->name, AST_TYPE_USER, size);
    return 1;
}

void append_left_child(ast_node *parent, const ast_node *child)
{
    parent->left = (ast_node *)child;
}
void append_right_child(ast_node *parent, const ast_node *child)
{
    parent->right = (ast_node *)child;
}
void append_middle_child(ast_node *parent, const ast_node *child)
{
    parent->middle = (ast_node *)child;
}

const ast_node *find_last_left_child(const ast_node *parent)
{
    ast_node *node = (ast_node *)parent;
    while (node->left)
    {
        node = node->left;
    }

    return node;
}
const ast_node *find_last_right_child(const ast_node *parent)
{
    ast_node *node = (ast_node *)parent;
    while (node->right)
    {
        node = node->right;
    }

    return node;
}
const ast_node *find_last_middle_child(const ast_node *parent)
{
    ast_node *node = (ast_node *)parent;
    while (node->middle)
    {
        node = node->middle;
    }

    return node;
}

ast_const_node *new_ast_int_const(int i)
{
    ast_const_node *n = IALLOC(ast_const_node);
    n->data.i = i;
    return n;
}
ast_const_node *new_ast_float_const(float f)
{
    ast_const_node *n = IALLOC(ast_const_node);
    n->data.f = f;
    return n;
}
ast_const_node *new_ast_str_const(const char *s)
{
    ast_const_node *n = IALLOC(ast_const_node);
    n->data.s = (char *)s;
    return n;
}

ast_identifier_node *new_identifier_node(symrec_t *symbol, type_t *type, int scope_level)
{
    ast_identifier_node *node = IALLOC(ast_identifier_node);
    node->scope_level = scope_level;
    node->sym = symbol;
    node->type = type;
    return node;
}
int get_scope_level()
{
    return scope_level;
}
void inc_scope_level()
{
    scope_level++;
}
void dec_scope_level()
{
    scope_level--;
}

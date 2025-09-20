#include "ast.h"
#include <stdlib.h>
#include <stdarg.h>

struct ast_node *new_ast_node(int code, ...)
{
    struct ast_node *temp;
    int i;
    int n = 0;
    va_list vl;
    va_start(vl, code);
    while (1)
    {
        temp = va_arg(vl, struct ast_node *);
        if (temp)
            n++;
        else
            break;
    }
    va_end(vl);

    struct ast_node **children = (struct ast_node **)malloc(sizeof(struct ast_node *) * n);

    va_start(vl, code);
    n = 0;

    while (1)
    {
        temp = va_arg(vl, struct ast_node *);
        if (temp)
        {
            children[n++] = temp;
        }
    }

    struct ast_node *g = (struct ast_node *)malloc(sizeof(struct ast_node));
    g->children = children;
    g->code = code;
    g->child_count = n;

    return g;
}

void append_child(struct ast_node *node, const struct ast_node *child)
{
    int n = node->child_count + 1;
    node->children = realloc(node->children, sizeof(struct ast_node *) * n);
    node->children[node->child_count] = child;
    node->child_count = n;
}
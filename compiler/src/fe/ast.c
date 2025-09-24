#include "ast.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

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

    struct ast_node *g = (struct ast_node *)malloc(sizeof(struct ast_node));
    g->children = 0;

    if (n > 0)
    {
        struct ast_node **children = (struct ast_node **)malloc(sizeof(struct ast_node *) * n);

        va_start(vl, code);
        i = 0;

        while (i < n)
        {
            temp = va_arg(vl, struct ast_node *);
            if (temp)
            {
                children[i++] = temp;
            }
        }

        g->children = children;
    }

    g->code = code;
    g->child_count = n;

    return g;
}

struct ast_node *new_ast_node_name(int code, const char *name, ...)
{

    struct ast_node *temp;
    int i;
    int n = 0;
    va_list vl;
    va_start(vl, name);
    while (1)
    {
        temp = va_arg(vl, struct ast_node *);
        if (temp)
            n++;
        else
            break;
    }
    va_end(vl);

    struct ast_node *g = (struct ast_node *)malloc(sizeof(struct ast_node));

    g->children = 0;

    if (n > 0)
    {
        struct ast_node **children = (struct ast_node **)malloc(sizeof(struct ast_node *) * n);

        va_start(vl, name);
        i = 0;

        while (i < n)
        {
            temp = va_arg(vl, struct ast_node *);
            if (temp)
            {
                children[i++] = temp;
            }
        }

        g->children = children;
        g->code = code;
    }

    g->child_count = n;

    g->data.str = strdup(name);
    return g;
}
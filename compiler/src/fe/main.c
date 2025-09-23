#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yyparse(struct ast_node **root);

void print_node(struct ast_node *node);

int main(void)
{
    struct ast_node *root;
    int ret;
    ret = yyparse(&root);
    if (!ret)
    {
        print_node(root);
    }

    exit(0);
}

void print_node(struct ast_node *node)
{
    if (node->child_count == 0)
    {
        printf("%d", node->code);
        if (node->data.str)
        {
            printf(" %s\n", node->data.str);
        }
        return;
    }

    printf("%d ", node->code);

    if (node->data.str)
    {
        printf("%s ", node->data.str);
    }

    for (int i = 0; i < node->child_count; i++)
    {
        struct ast_node *n = node->children[i];
        print_node(n);
    }
}
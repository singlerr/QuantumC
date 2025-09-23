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
        printf("%d\n", node->code);
        return;
    }

    printf("%d ", node->code);
    for (int i = 0; i < node->child_count; i++)
    {
        struct ast_node *n = node->children[i];
        print_node(n);
    }
}
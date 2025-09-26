#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern FILE *yyin;
extern int yyparse(struct ast_node **root);

void print_node(struct ast_node *node);

int main(int argc, char *argv[])
{
    struct ast_node *root;
    FILE *f;
    int ret;
    if (argc > 1)
    {
        if ((f = fopen(argv[1], "r")) == 0)
        {
            fprintf(stderr, "file open error for %s\n", argv[1]);
            exit(1);
        }

        yyin = f;
    }

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
        printf("%s ", to_ast_string(node->code));
        if (node->data.str)
        {
            printf(" %s\n", node->data.str);
        }
        return;
    }

    printf("%s ", to_ast_string(node->code));

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
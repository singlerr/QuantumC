#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symrec.h"
extern FILE *yyin;
extern int yyparse(ast_node **root);

void print_node(ast_node *node);

int main(int argc, char *argv[])
{

    ast_node *root;
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

    init_type();

    ret = yyparse(&root);
    if (!ret)
    {
        print_node(root);
    }

    exit(0);
}

void print_node(ast_node *node)
{
}
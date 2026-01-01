#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "symrec.h"
#include "ast_sqz.h"

extern FILE *yyin;
extern int yyparse(ast_node **root);
extern int squeeze_ast(ast_node *program, sqz_program **out);
void print_node(ast_node *node);
void print_node_recursion(ast_node *node, int depth);
char *yyfilename;

int main(int argc, char *argv[])
{

    ast_node *root;
    sqz_program *squeezed;
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
        yyfilename = argv[1];
    }

    init_type();

    ret = yyparse(&root);
    if (ret)
    {
        exit(0);
    }

    if (FAILED(squeeze_ast(root, &squeezed)))
    {
        exit(1);
    }

    exit(0);
}

void print_node(ast_node *node)
{
    if (node == NULL)
    {
        fprintf(stdout, "The given AST is empty.\n");
    }
    else
    {
        print_node_recursion(node, 0);
    }

    return;
}
void print_node_recursion(ast_node *node, int depth)
{
    for (int i = 0; i < depth; i++)
    {
        fprintf(stdout, "  ");
    }

    char *node_identifier_str = node->identifier != NULL ? node->identifier->sym->name : "N/A";
    char *node_type_str = node->type != NULL ? node->type->name : "N/A";

    fprintf(stdout, "ID: %s (TYPE: %s)\n", node_identifier_str, node_type_str);

    if (node->left != NULL)
        print_node_recursion(node->left, depth + 1);
    if (node->middle != NULL)
        print_node_recursion(node->middle, depth + 1);
    if (node->right != NULL)
        print_node_recursion(node->right, depth + 1);

    return;
}

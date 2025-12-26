#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symrec.h"
extern FILE *yyin;
extern int yyparse(ast_node **root);

void print_node(ast_node *node);
void print_node_recursion(ast_node *node, int depth) 

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

    char *node_identifier_str = node->identifier != NULL ? node->identifier->type->name : "N/A";
    char *node_type_str = node->type->name;

    fprintf(stdout, "ID: %s (TYPE: %s)\n", node_identifier_str, node_type_str);

    if (node->left != NULL) print_node_recursion(node->left, depth+1);
    if (node->middle != NULL) print_node_recursion(node->middle, depth+1);
    if (node->right != NULL) print_node_recursion(node->right, depth+1);

    return;
}

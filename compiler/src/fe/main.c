#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "symrec.h"
#include "ast_sqz.h"
#include "ast_sem.h"

extern FILE *yyin;
extern int yyparse(ast_node **root);
extern int squeeze_ast(ast_node *program, sqz_program **out);

static void print_indent(int depth);

static void print_ast_rec(ast_node *node, int depth);
void print_ast(ast_node *node);

static void print_sqz_var_decl(sqz_var_decl *v);
static void print_sqz_func_decl(sqz_func_decl *f);
static void print_sqz_decl_list(sqz_decl *decl, int depth);
void print_sqz(sqz_program *program);

void free_ast(ast_node *root);
void free_sqz(sqz_program *program);

char *yyfilename;

int main(int argc, char *argv[])
{
    ast_node *root;
    sqz_program *squeezed;
    struct sem_program *sem_analysis;
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

    if (FAILED(sem_program(squeezed, &sem_analysis)))
    {
        exit(1);
    }
    print_sqz(squeezed);

    free_ast(root);
    free_sqz(squeezed);

    exit(0);
}

static void print_indent(int depth)
{
    for (int i = 0; i < depth; i++)
    {
        fprintf(stdout, "    ");
    }

    return;
}

static void print_ast_rec(ast_node *node, int depth)
{
    const char *name = "N/A";
    const char *type = "N/A";

    if (node->identifier && node->identifier->type)
    {
        name = node->identifier->type->name;
    }
    if (node->type)
    {
        type = node->type->name;
    }

    print_indent(depth);
    fprintf(stdout, "ID: %s (TYPE: %s)\n", name, type);

    if (node->left)
    {
        print_ast_rec(node->left, depth + 1);
    }
    if (node->middle)
    {
        print_ast_rec(node->middle, depth + 1);
    }
    if (node->right != NULL)
    {
        print_ast_rec(node->right, depth + 1);
    }

    return;
}

void print_ast(ast_node *node)
{
    if (node == NULL)
    {
        fprintf(stdout, "ERROR: The given AST is empty.\n");
    }
    else
    {
        print_ast_rec(node, 0);
    }

    return;
}

static void print_sqz_var_decl(sqz_var_decl *v)
{
    if (!v)
    {
        return;
    }

    for (sqz_init_decl *id = v->decl_list; id; id = id->next)
    {
        const char *name = "N/A";
        const char *type = "N/A";

        if (id->decl && id->decl->id && id->decl->id->name)
        {
            name = id->decl->id->name->name;
        }
        if (id->decl && id->decl->type && id->decl->type->name)
        {
            type = id->decl->type->name;
        }

        printf("VAR: %s (type: %s)\n", name, type);
    }

    return;
}

static void print_sqz_func_decl(sqz_func_decl *f)
{
    if (!f)
    {
        return;
    }

    const char *type = "N/A";

    if (f->return_type && f->return_type->name)
    {
        type = f->return_type->name;
    }

    printf("FUNC (return type: %s)\n", type);

    return;
}

static void print_sqz_decl_list(sqz_decl *decl, int depth)
{
    for (sqz_decl *d = decl; d != NULL; d = d->next)
    {
        print_indent(depth);

        if (d->decl_type == AST_VARIABLE_DECLARATION)
        {
            print_sqz_var_decl(d->decl.var);
        }
        else if (d->decl_type == AST_FUNCTION_DECLARATION)
        {
            print_sqz_func_decl(d->decl.func);
        }
        else
        {
            printf("DECL (type = %d)\n", d->decl_type);
        }
    }

    return;
}

void print_sqz(sqz_program *program)
{
    if (program == NULL || program->decl == NULL)
    {
        fprintf(stdout, "ERROR: The given program is empty.\n");
        return;
    }

    print_sqz_decl_list(program->decl, 0);

    return;
}

void free_ast(ast_node *root)
{
    if (!root)
    {
        fprintf(stderr, "ERROR: The given AST pointer is invalid.\n");
        return;
    }

    if (root->left)
    {
        free_ast(root->left);
    }
    if (root->middle)
    {
        free_ast(root->middle);
    }
    if (root->right)
    {
        free_ast(root->right);
    }

    free(root);

    return;
}

void free_sqz(sqz_program *program)
{
    if (!program)
    {
        fprintf(stderr, "ERROR: The given squeezed program pointer is invalid.\n");
        return;
    }

    sqz_decl *curr = program->decl;
    while (!curr)
    {
        sqz_decl *temp = curr->next;
        free(curr);
        curr = temp;
    }

    return;
}

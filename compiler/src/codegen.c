#include "codegen.h"
#include "ast_sem.h"
#include "ast_typing.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "diagnostics.h"

FILE *fout = NULL;

void gen_str();
void gen_int();
void gen_float();
void gen_double();
void gen_short();
void gen_char();
void gen_type(const type_t *);
void gen_var_declaration(struct sem_var_decl *var);
void gen_func_declaration(struct sem_func_decl *func);
void newline();
void space();

void set_codegen_output(FILE *out)
{
    fout = out;
}

void gen_program(struct sem_program *prog)
{
    struct sem_decl *d = prog->decl;
    while (d)
    {
        switch (d->decl_type)
        {
        case AST_FUNCTION_DECLARATION:
            gen_func_declaration(d->decl.func);
            break;
        case AST_VARIABLE_DECLARATION:
            gen_var_declaration(d->decl.var);
            break;
        }
        d = d->next;
    }
}

void gen_var_declaration(struct sem_var_decl *var)
{
}
void gen_func_declaration(struct sem_func_decl *func)
{
}

void gen_type(const type_t *type)
{
    // classical
    if (IS_INT(type))
    {
        int bit = IS_INT64(type) ? 64 : 32;
        fprintf(fout, "int[%d]", bit);
    }
    else if (IS_FLOAT(type))
    {
        int bit = IS_FLOAT64(type) ? 64 : 32;
        fprintf(fout, "float[%d]", bit);
    }
    else if (IS_COMPLEX(type))
    {
        fprintf(fout, "complex");
    }
    // quantum types
    else if (IS_QUBIT(type))
    {
        fprintf(fout, "qubit");
    }
    else if (IS_ANGLE(type))
    {
        fprintf(fout, "angle");
    }
    else
    {
        fprintf(stderr, "Error: Unsupported type %s", type->name);
        exit(1);
    }
}

void newline()
{
    fprintf(fout, "\n");
}
void space()
{
    fprintf(fout, " ");
}

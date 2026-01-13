#include "codegen.h"
#include "ast_sem.h"
#include "ast_typing.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "diagnostics.h"

FILE *fout = NULL;

void gen(const char *msg, ...);
void error(const char *msg, ...);

void gen_type(const type_t *);
void gen_var_declaration(struct sem_var_decl *var);
void gen_func_declaration(struct sem_func_decl *func);
void gen_declarator(struct sem_declarator *declarator);
void gen_decl_spec(struct sem_decl_spec *spec);
void gen_initializer(struct sem_initializer *init);
void gen_assign_expr(struct sem_assign_expr *expr);
void gen_unary_expr(struct sem_unary *unary);
void gen_ternary_expr(struct sem_ternary_expr *ternary);
void gen_binary_expr(struct sem_binary_expr *binary);
void gen_operator(ast_node_type op_type);
void gen_cast_expr(struct sem_cast_expr *cast);
void gen_postfix(struct sem_expr_src *postfix);

void begin_paren();
void end_paren();
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
    struct sem_init_decl *d = var->decl_list;

    while (d)
    {
        struct sem_declarator *declarator = d->decl;
        struct sem_initializer *initializer = d->init;
        d = d->next;
    }
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
        gen("int[%d]", bit);
    }
    else if (IS_FLOAT(type))
    {
        int bit = IS_FLOAT64(type) ? 64 : 32;
        gen("float[%d]", bit);
    }
    else if (IS_COMPLEX(type))
    {
        gen("complex");
    }
    // quantum types
    else if (IS_QUBIT(type))
    {
        gen("qubit");
    }
    else if (IS_ANGLE(type))
    {
        gen("angle");
    }
    else
    {
        error("Error: Unsupported type %s", type->name);
        exit(1);
    }
}

void gen_declarator(struct sem_declarator *declarator)
{
    if (declarator->index)
    {
        gen("[");
    }
}

void gen_decl_spec(struct sem_decl_spec *spec)
{
    if (spec->qualifier & QAL_CONST)
    {
        gen("const");
        space();
    }
}

void gen_initializer(struct sem_initializer *init)
{
}

void gen_assign_expr(struct sem_assign_expr *expr)
{
    switch (expr->assign_type)
    {
    case AST_EXPR_ASSIGN:
        struct sem_unary *left = expr->left;
        struct sem_assign_expr *right = expr->right;

        gen_unary_expr(left);
        space();
        gen("=");
        space();
        gen_assign_expr(right);
        break;

    default:
        gen_ternary_expr(expr->ternary_expr);
        break;
    }
}

void gen_unary_expr(struct sem_unary *unary)
{
    switch (unary->expr_type)
    {
    case AST_EXPR_PRE_INC:
    case AST_EXPR_PRE_DEC:
        perror("pre inc or pre dec are not supported");
        break;
    case AST_UNARY_AMP:
    case AST_UNARY_STAR:
    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_TILDE:
    case AST_UNARY_EXCL:
        gen_operator(unary->expr_type);
        space();
        gen_unary_expr(unary->expr.cast);
        break;
    default:
        gen_postfix(unary->expr.postfix);
        break;
    }
}

void gen_postfix(struct sem_expr_src *postfix)
{
    switch (postfix->expr_type)
    {
    case AST_EXPR_ARRAY_ACCESS:
        /* code */
        break;
    case AST_EXPR_FUNCTION_CALL:
        break;
    case AST_EXPR_MEMBER_ACCESS:
        break;
    case AST_ default:
        break;
    }
}

void gen_ternary_expr(struct sem_ternary_expr *ternary)
{
    if (ternary->binary_expr)
    {
        gen_binary_expr(ternary->binary_expr);
    }
    else
    {
        perror("OpenQASM does not have ternary expression");
    }
}

void gen_binary_expr(struct sem_binary_expr *binary)
{
    switch (binary->expr_type)
    {
    case AST_EXPR_LOR:
    case AST_EXPR_LAND:
    case AST_EXPR_OR:
    case AST_EXPR_XOR:
    case AST_EXPR_AND:
    case AST_EXPR_EQ:
    case AST_EXPR_NEQ:
    case AST_EXPR_LT:
    case AST_EXPR_GT:
    case AST_EXPR_LEQ:
    case AST_EXPR_GEQ:
    case AST_EXPR_LSHIFT:
    case AST_EXPR_RSHIFT:
    case AST_EXPR_ADD:
    case AST_EXPR_SUB:
        gen_binary_expr(binary->left);
        space();
        gen_operator(binary->expr_type);
        space();
        gen_binary_expr(binary->right.binary);
        break;
    case AST_EXPR_MUL:
    case AST_EXPR_DIV:
    case AST_EXPR_MOD:
        gen_binary_expr(binary->left);
        space();
        gen_operator(binary->expr_type);
        space();
        gen_cast_expr(binary->right.cast);
        break;
    case AST_EXPR_TYPE_CAST:
        gen_cast_expr(binary->cast_expr);
        break;
    default:
        perror("Unknown binary expression");
        break;
    }
}

void gen_cast_expr(struct sem_cast_expr *cast)
{
    switch (cast->cast_type)
    {
    case AST_EXPR_TYPE_CAST:
        begin_paren();
        gen_declarator(cast->type);
        end_paren();
        gen_cast_expr(cast->expr.cast);
        break;
    default:
        gen_unary_expr(cast->expr.unary);
        break;
    }
}

void gen_operator(ast_node_type op_type)
{
    char *op;
    switch (op_type)
    {
    case AST_EXPR_LOR:
        op = "||";
        break;
    case AST_EXPR_LAND:
        op = "&&";
        break;
    case AST_EXPR_OR:
        op = "|";
        break;
    case AST_EXPR_XOR:
        op = "^";
        break;
    case AST_UNARY_AMP:
    case AST_EXPR_AND:
        op = "&";
        break;
    case AST_EXPR_EQ:
        op = "==";
        break;
    case AST_EXPR_NEQ:
        op = "!=";
        break;
    case AST_EXPR_LT:
        op = "<";
        break;
    case AST_EXPR_GT:
        op = ">";
        break;
    case AST_EXPR_LEQ:
        op = "<=";
        break;
    case AST_EXPR_GEQ:
        op = ">=";
        break;
    case AST_EXPR_LSHIFT:
        op = "<<";
        break;
    case AST_EXPR_RSHIFT:
        op = ">>";
        break;
    case AST_UNARY_PLUS:
    case AST_EXPR_ADD:
        op = "+";
        break;
    case AST_UNARY_MINUS:
    case AST_EXPR_SUB:
        op = "-";
        break;
    case AST_UNARY_STAR:
    case AST_EXPR_MUL:
        op = "*";
        break;
    case AST_EXPR_DIV:
        op = "/";
        break;
    case AST_EXPR_MOD:
        op = "%";
        break;
    case AST_UNARY_TILDE:
        op = "~";
        break;
    case AST_UNARY_EXCL:
        op = "!";
        break;
    default:
        perror("Unknown operation type");
        break;
    }

    gen("%s", op);
}

void newline()
{
    fprintf(fout, "\n");
}
void space()
{
    fprintf(fout, " ");
}

void begin_paren()
{
    gen("(");
}
void end_paren()
{
    gen(")");
}

void gen(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(fout, msg, args);
    va_end(args);
}

void error(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}
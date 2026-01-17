

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "diagnostics.h"
#include "codegen.h"
#include "ast_sem.h"
#include "ast_typing.h"
FILE *fout = NULL;

void gen(const char *msg, ...);
void error(const char *msg, ...);

void gen_type(const type *);
void gen_operator(operator op_type);
void gen_statement(statement *stmt);

void begin_paren();
void end_paren();
void newline();
void space();

void set_codegen_output(FILE *out)
{
    fout = out;
}

void gen_program(struct program *prog)
{
    statement_list *list = prog->stmts;

    statement_list *cur;
    list_for_each_entry(cur, list)
    {
        gen_statement(cur->value);
    }
}

void gen_statement(statement *stmt)
{
    switch (stmt->kind)
    {
    case STMT_CLASSICAL_DECLARATION:
        break;

    default:
        break;
    }
}

void gen_type(const type *type)
{
    gen(type->type_name);
}

void gen_operator(operator op_type)
{
    const char *op = NULL;

    switch (op_type)
    {
    case OP_DOUBLE_ASTERISK:
        op = "**";
        break;
    case OP_TILDE:
        op = "~";
        break;
    case OP_EXCLAMATION_POINT:
        op = "!";
        break;
    case OP_MINUS:
        op = "-";
        break;
    case OP_ASTERISK:
        op = "*";
        break;
    case OP_SLASH:
        op = "/";
        break;
    case OP_PERCENT:
        op = "%";
        break;
    case OP_PLUS:
        op = "+";
        break;
    case OP_LSHIFT:
        op = "<<";
        break;
    case OP_RSHIFT:
        op = ">>";
        break;
    case OP_LT:
        op = "<";
        break;
    case OP_GT:
        op = ">";
        break;
    case OP_GEQ:
        op = ">=";
        break;
    case OP_LEQ:
        op = "<=";
        break;
    case OP_EQ:
        op = "==";
        break;
    case OP_NEQ:
        op = "!=";
        break;
    case OP_AMP:
        op = "&";
        break;
    case OP_PIPE:
        op = "|";
        break;
    case OP_CARET:
        op = "^";
        break;
    case OP_DOUBLE_AMP:
        op = "&&";
        break;
    case OP_DOUBLE_PIPE:
        op = "||";
        break;

    default:
        P_ERROR("Unknown operator");
        return;
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


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "diagnostics.h"
#include "codegen.h"
#include "ast_sem.h"
#include "ast_typing.h"

#define GEN_SIZE(type)        \
    do                        \
    {                         \
        begin_bracket();      \
        gen_expr(type->size); \
        end_bracket();        \
    } while (0)

FILE *fout = NULL;

int indent = 0;
void gen(const char *msg, ...);
void error(const char *msg, ...);

void gen_array_type(const array_type *);
void gen_scalar_type(const classical_type *);
void gen_type(const type *);
void gen_classical_type(const classical_type *);
void gen_quantum_type(const quantum_type *);
void gen_operator(const operator op_type);
void gen_statement(const statement *stmt);
void gen_identifier(const identifier *id);
void gen_expr(const expression *expr);
void gen_expr_list(const expression_list *expr_list);
void gen_indent();
void begin_brace();
void end_brace();
void begin_bracket();
void end_bracket();
void begin_paren();
void end_paren();
void comma();
void newline();
void space();
void end_stmt();
void assign();

static inline void gen_indexed_identifier(indexed_identifier *id)
{
    gen_identifier(id->name);
    begin_bracket();
    gen_expr(id->index->index.expr_or_range->expr);
    end_bracket();
}

static inline void gen_qubit(qubit *qubit)
{
    switch (qubit->kind)
    {
    case ID_INDEXED_IDENTIFIER:
        gen_indexed_identifier(qubit->value.indexed_identifier);
        break;
    case ID_IDENTIFIER:
        gen_identifier(qubit->value.identifier);
        break;
    }
}

static inline void gen_statement_list(statement_list *list)
{
    statement_list *s;
    list_for_each_entry(s, list)
    {
        gen_statement(s->value);
    }
}

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

void gen_statement(const statement *stmt)
{

    switch (stmt->kind)
    {
    case STMT_CLASSICAL_DECLARATION:
        gen_classical_type(stmt->classical.declaration.type);
        space();
        gen_identifier(stmt->classical.declaration.identifier);

        if (stmt->classical.declaration.init_expression_kind == EXPR_EXPRESSION)
        {
            assign();
            gen_expr(stmt->classical.declaration.init_expression.expr);
        }
        end_stmt();
        break;
    case STMT_QUANTUM_DECLARATION:
        gen("qubit");
        begin_bracket();
        gen_expr(stmt->classical.qubit_declaration.size);
        end_bracket();
        space();
        gen_identifier(stmt->classical.qubit_declaration.qubit);
        end_stmt();
        break;
    case STMT_DEF:
        cls_or_quantum_args_list *arg_list;
        gen("def");
        space();
        gen_identifier(stmt->classical.subroutine_definition.name);
        begin_paren();

        list_for_each_entry(arg_list, stmt->classical.subroutine_definition.arguments)
        {
            switch (arg_list->value->kind)
            {
            case CLASSICAL_ARGUMENT:
                gen_classical_type(arg_list->value->classical_argument->type);
                space();
                gen_identifier(arg_list->value->classical_argument->name);
                break;
            case QUANTUM_ARGUMENT:
                P_ERROR("Quantum argument is implemented");
            }

            if (arg_list->next)
            {
                comma();
            }
        }
        end_paren();
        space();
        gen("->");
        space();
        gen_classical_type(stmt->classical.subroutine_definition.return_type);
        space();
        begin_brace();
        newline();
        gen_statement_list(stmt->classical.subroutine_definition.body);
        end_brace();
        break;
    case STMT_SWITCH:
        gen("switch");
        space();
        begin_paren();
        gen_expr(stmt->classical.swtch.target);
        end_paren();
        space();
        begin_brace();
        newline();

        case_stmt_list *case_stmt;
        list_for_each_entry(case_stmt, stmt->classical.swtch.cases)
        {
            gen("case");
            space();
            gen_expr_list(case_stmt->value->expr);
            space();
            begin_brace();
            newline();
            gen_statement(case_stmt->value->statmenet);
            end_brace();
            newline();
        }
        end_brace();
        break;
    case STMT_WHILE:
        gen("while");
        space();
        begin_paren();
        gen_expr(stmt->classical.while_loop.condition);
        end_paren();
        begin_brace();
        newline();
        gen_statement_list(stmt->classical.while_loop.block);
        end_brace();
        break;
    case STMT_COMPOUND:
        gen_statement_list(stmt->classical.compound.statements);
        break;
    case STMT_EXPRESSION:
        gen_expr(stmt->classical.expression.expr);
        end_stmt();
        break;
    case STMT_CONTINUE:
        gen("continue");
        end_stmt();
        break;
    case STMT_BREAK:
        gen("break");
        end_stmt();
        break;
    case STMT_RETURN:
        gen("return");
        space();
        switch (stmt->classical.retrn.kind)
        {
        case EXPR_EXPRESSION:
            if (stmt->classical.retrn.expr.expr)
            {
                gen_expr(stmt->classical.retrn.expr.expr);
            }
            break;
        default:
            P_ERROR("Measure expression in return is not implemented");
        }

        end_stmt();
        break;
    case STMT_IF:
        gen("if");
        space();
        begin_paren();
        gen_expr(stmt->classical.branching.condition);
        end_paren();
        begin_brace();
        newline();
        gen_statement_list(stmt->classical.branching.if_block);
        end_brace();
        if (stmt->classical.branching.else_block)
        {
            space();
            gen("else");
            space();
            begin_brace();
            newline();
            gen_statement_list(stmt->classical.branching.else_block);
            end_brace();
        }
        break;
    default:
        P_ERROR("Statement %d is not implemented", stmt->kind);
        break;
    }

    newline();
}

void gen_type(const type *type)
{
    if (type->kind == CLASSICAL_TYPE)
    {
        gen_classical_type(type->classical_type);
    }
    else if (type->kind == QUANTUM_TYPE)
    {
        gen_quantum_type(type->quantum_type);
    }
}

void gen_classical_type(const classical_type *cls_type)
{
    if (cls_type->kind == TYPE_ARRAY)
    {
        gen_array_type(cls_type->array_type);
    }
    else
    {
        gen_scalar_type(cls_type);
    }
}

void gen_quantum_type(const quantum_type *q_type)
{
    gen(q_type->type_name);
    switch (q_type->kind)
    {
    case TYPE_QUBIT:
        GEN_SIZE(q_type->qubit_type);
        break;
    default:
        P_ERROR("Unknown type: %s", q_type->type_name);
        break;
    }
}

void gen_array_type(const array_type *type)
{
    gen("array");
    begin_bracket();
    gen_type(type->base_type);
    comma();
    switch (type->dimension_kind)
    {
    case SINGLE_EXPRESSION:
        gen_expr(type->dimensions.expr);
        break;
    case EXPRESSION_LIST:
        gen_expr_list(type->dimensions.expr_list);
        break;
    }
    end_bracket();
}

void gen_scalar_type(const classical_type *type)
{
    gen(type->type_name);
    switch (type->kind)
    {
    case TYPE_INT:
        GEN_SIZE(type->int_type);
        break;
    case TYPE_UINT:
        GEN_SIZE(type->uint_type);
        break;
    case TYPE_FLOAT:
        GEN_SIZE(type->float_type);
        break;
    case TYPE_ANGLE:
        GEN_SIZE(type->angle_type);
        break;
    case TYPE_BIT:
        GEN_SIZE(type->bit_type);
        break;
    case TYPE_BOOL:
        GEN_SIZE(type->bool_type);
        break;
    case TYPE_DURATION:
        break;
    case TYPE_COMPLEX:
        gen("float");
        GEN_SIZE(type->complex_type->base_type);
        break;
    default:
        P_ERROR("Unknown type: %s", type->type_name);
        break;
    }
}

void gen_identifier(const identifier *id)
{
    gen(id->name);
}

void gen_expr(const expression *expr)
{
    switch (expr->kind)
    {
    case EXPR_UNARY:
        gen_expr(expr->as.unary.expr);
        gen_operator(expr->as.unary.op);
        break;
    case EXPR_BINARY:
        gen_expr(expr->as.binary.lhs);
        space();
        gen_operator(expr->as.binary.op);
        space();
        gen_expr(expr->as.binary.rhs);
        break;
    case EXPR_LITERAL:
    {
        switch (expr->as.literal.literal_kind)
        {
        case LIT_BIN_INT:
            gen("0b%b", expr->as.literal.data.i);
            break;
        case LIT_OCT_INT:
            gen("0o%o", expr->as.literal.data.i);
            break;
        case LIT_DEC_INT:
            gen("%d", expr->as.literal.data.i);
            break;
        case LIT_HEX_INT:
            gen("0x%x", expr->as.literal.data.i);
            break;
        case LIT_FLOAT:
            gen("%f", expr->as.literal.data.f);
            break;
        case LIT_IMAGINARY:
            P_ERROR("Imaginary expression is not implemented");
            break;
        case LIT_BOOL:
            gen("%d", expr->as.literal.data.b);
            break;
        case LIT_BIT_STR:
            gen("\"%s\"", expr->as.literal.data.bit_str);
            break;
        case LIT_TIMING:
            P_ERROR("Timing expression is implemented");
            break;
        case LIT_IDENTIFIER:
            gen("%s", expr->as.literal.data.identifier);
            break;
        default:
            P_ERROR("Unknown literal type");
            break;
        }
    }
    break;
    case EXPR_FUNC_CALL:
        gen_identifier(expr->as.function_call.name);
        begin_paren();
        gen_expr_list(expr->as.function_call.arguments);
        end_paren();
        break;
    case EXPR_CAST:
        begin_paren();
        gen_classical_type(expr->as.cast.type);
        end_paren();
        gen_expr(expr->as.cast.argument);
        break;
    case EXPR_IDENTIFIER:
        gen_identifier(expr->as.identifier);
        break;
    case EXPR_INDEX:
        expr_or_range_list *idx;
        expr_or_range_list *index = expr->as.index.list;
        list_goto_first(expr_or_range_list, index);
        gen_expr(expr->as.index.collection);
        begin_bracket();
        list_for_each_entry(idx, index)
        {
            gen_expr(idx->value->expr);

            if (idx->next)
            {
                comma();
                space();
            }
        }
        end_bracket();
        break;
    case EXPR_QUANTUM_MEASUREMENT:
        gen("measure");
        space();
        gen_qubit(expr->as.quantum_measurement.measure.qubit);
        break;
    default:
        P_ERROR("Expression %d is not implemented", expr->kind);
        break;
    }
}

void gen_expr_list(const expression_list *expr_list)
{
    expression_list *expr;

    list_for_each_entry(expr, expr_list)
    {
        gen_expr(expr->value);

        if (expr->next)
        {
            comma();
            space();
        }
    }
}

void gen_operator(const operator op_type)
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
    case OP_ASSIGN:
        op = "=";
        break;
    case OP_MUL_ASSIGN:
        op = "*=";
        break;
    case OP_DIV_ASSIGN:
        op = "/=";
        break;
    case OP_MOD_ASSIGN:
        op = "%=";
        break;
    case OP_PLUS_ASSIGN:
        op = "+=";
        break;
    case OP_MINUS_ASSIGN:
        op = "-=";
        break;
    case OP_LSHIFT_ASSIGN:
        op = "<<=";
        break;
    case OP_RSHIFT_ASSIGN:
        op = ">>=";
        break;
    case OP_AND_ASSIGN:
        op = "&=";
        break;
    case OP_XOR_ASSIGN:
        op = "^=";
        break;
    case OP_OR_ASSIGN:
        op = "|=";
        break;
    default:
        P_ERROR("Unknown operator %d", op_type);
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

void begin_brace()
{
    gen("{");
    indent++;
}
void end_brace()
{
    gen("}");
    indent--;
}

void begin_bracket()
{
    gen("[");
}
void end_bracket()
{
    gen("]");
}

void comma()
{
    gen(",");
}

void end_stmt()
{
    gen(";");
}

void assign()
{
    gen("=");
}

void gen_indent()
{
    for (int i = 0; i < indent; i++)
    {
        fprintf(stdout, "\t");
    }
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
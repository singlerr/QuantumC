#ifndef _AST_SEM_H_
#define _AST_SEM_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"

typedef enum operator
{
    OP_DOUBLE_ASTERISK,
    OP_TILDE,
    OP_EXCLAMATION_POINT,
    OP_MINUS,
    OP_ASTERISK,
    OP_SLASH,
    OP_PERCENT,
    OP_PLUS
} operator;

typedef enum expr_kind
{
    EXPR_INDEX,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CAST,
    EXPR_CALL,
    EXPR_DURATIONOF,
    EXPR_PRIMARY
} expr_kind;

typedef enum stmt_kind
{
    ALIAS_DECLARATION,
    ASSIGNMENT,
    BARRIER,
    BOX,
    BREAK,
    CAL,
    CALIBRATION_GRAMMAR,
    CLASSICAL_DECLARATION,
    CONST_DECLARATION,
    CONTINUE,
    DEF,
    DEFCAL,
    DELAY,
    END,
    EXPRESSION,
    EXTERN,
    FOR,
    GATE_CALL,
    GATE,
    IF,
    INCLUDE,
    IO_DECLARATION,
    MEASURE_ARROW_ASSIGNMENT,
    NOP,
    OLD_STYLE_DECLARATION,
    QUANTUM_DECLARATION,
    RESET,
    RETURN,
    SWITCH,
    WHILE
} stmt_kind;

typedef struct
{
    stmt_kind kind;

    union
    {
        struct
        {

        } assignment;
    } st;
} type;

typedef struct statement
{

} statement;

typedef struct expression
{
    expr_kind kind;

    struct expression *next;

    union
    {
        struct
        {
            struct
            {

            } operator;
            struct expression *expr;

        } index;

        struct
        {
            operator op;
            struct expression *expr;
        } unary;

        struct
        {
            operator op;
            struct expression *left;
            struct expression *right;
        } binary;

        struct
        {
            type type;
            struct expression *expr;
        } cast;

        struct
        {
            struct expression *head;
            struct expression *tail;
        } call;

        struct
        {
            statement *statement;
        } durationof;

        struct
        {
            enum
            {
                BIN_INT,
                OCT_INT,
                DEC_INT,
                HEX_INT,
                FLOAT,
                IMAGINARY,
                BOOL,
                BIT_STR,
                TIMING,
                HWQUBIT,
                IDENTIFIER
            } literal_kind;

            union
            {
                char *identifier;
                int i;
                float f;
                BOOL b;
                char *bit_str;
                int timing;
            } data;

        } primary;
    } as;
} expression;

#endif
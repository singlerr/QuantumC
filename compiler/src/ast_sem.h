#ifndef _AST_SEM_H_
#define _AST_SEM_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"
#include "list.h"

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
    EXPR_PRIMARY,
    EXPR_RANGED
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

typedef struct symbol_t
{
    char *name;
} symbol_t;

typedef struct
{

} type;

typedef struct
{

} classical_type;

typedef struct identifier
{
    char *name;
} identifier;

typedef struct annotation
{
    struct list_head list;

    char *keyword;
    char *command;
} annotation;

typedef struct discrete_set
{
    struct list_head values; /* Expression */
} discrete_set;

typedef struct range_definition
{
    struct expression *start;
    struct expression *end;
    struct expression *step;
} range_definition;

typedef struct index_element
{
    enum
    {
        DISCRETE_SET,
        EXPRESSION,
        RANGED_DEFINITION
    } kind;
    union
    {
        discrete_set *discrete_set;
        struct list_head expr;
    } index;
} index_element;

typedef struct indexed_identifier
{
    identifier *name;
    struct list_head indices; /* Index Element */
} indexed_identifier;

typedef struct quantum_gate_modifier
{
    enum
    {
        INV,
        POW,
        CTRL,
        NEGCTRL
    } gate_modifier_name;
    struct expression *argument;
} quantum_gate_modifier;

typedef struct qubit
{
    enum
    {
        INDEXED_IDENTIFIER,
        IDENTIFIER
    } kind;

    union
    {
        indexed_identifier *indexed_identifier;
        identifier *identifier;
    } value;

} qubit;

typedef struct quantum_measurement
{
    qubit *qubit;
} quantum_measurement;

typedef struct quantum_argument
{
    identifier *name;
    struct expression *size;
} quantum_argument;

typedef struct statement
{
    stmt_kind kind;
    struct list_head annotations;

    union
    {

        struct
        {
            struct list_head statements;
        } compound;

        struct
        {
            char *filename;
        } include;

        struct
        {
            struct expression *expr;
        } expression;

        struct
        {
            identifier *qubit;
            struct expression *size;
        } qubit_declaration;

        struct
        {
            identifier *name;
            struct list_head arguments; /* Identifier */
            struct list_head qubits;    /* Identifier */
            struct list_head body;      /* Quantum Statement */
        } quantum_gate_definition;

        struct
        {
            identifier *name;
            struct list_head arguments; /* Extern Argument */
            classical_type *return_type;
        } extern_declaration;

        struct
        {
            classical_type *type;
            identifier *identifier;
            enum
            {
                EXPRESSION,
                QUANTUM_MEASUREMENT
            } init_expression_kind;
            union
            {
                struct expression *expr;
                quantum_measurement *measurment;
            } init_expression;
        } declaration;

        struct
        {
            enum
            {
                INPUT,
                OUTPUT
            } io_keyword;

            classical_type *type;
            identifier *identifier;
        } io_declaration;

        struct
        {
            classical_type *type;
            identifier *identifier;
            struct expression *expression;
        } constant_declaration;

        struct
        {
            char *name;
        } calbiration_grammar_declaration;

        struct
        {
            char *body;
        } calibration;

        struct
        {
            identifier *name;
            struct list_head arguments;
            struct list_head qubits;
            classical_type *return_type;
            char *body;
        } calibration_definition;

        struct
        {
            identifier *name;
            struct list_head arguments;
            struct list_head body;
            classical_type *return_type;
        } subroutine_definition;

        struct
        {
            enum
            {
                EXPRESSION,
                QUANTUM_MEASUREMENT
            } expr_kind;
            union
            {
                struct expression *expr;
                quantum_measurement *measurement;
            } expr;
        } retrn;

        struct
        {
            struct expression *condition;
            struct list_head if_block;
            struct list_head else_block;
        } branching;

        struct
        {
            struct expression *condition;
            struct list_head block;
        } while_loop;

        struct
        {
            classical_type *type;
            identifier *identifier;
            enum
            {
                RANGED_DEFINITION,
                DISCRETE_SET,
                EXPRESSION
            } set_declaration_kind;
            union
            {
                range_definition *range_definition;
                discrete_set *discrete_set;
                struct expression *expression;
            } set_declaration;
            struct list_head block;
        } for_in_loop;

        struct
        {
            struct expression *target;
            struct list_head cases;
            struct statement *deflt;
        } swtch;

        struct
        {
            struct expression *duration;
            struct list_head qubits;
        } delay;

        struct
        {
            struct expression *duration;
            struct list_head body;
        } box;

        struct
        {
            struct list_head target;
        } duration_of;

        struct
        {
            struct expression *target;
            struct expression *index;
        } size_of;

        struct
        {
            identifier *target;
            enum
            {
                IDENTIFIER,
                CONCAT
            } value_kind;
            union
            {
                identifier *identifier;
                struct expression *concat;
            } value;

        } alias;

        struct
        {
            enum
            {
                IDENTIFIER,
                INDEXED_IDENTIFIER
            } lvalue_kind;
            union
            {
                identifier *identifier;
                indexed_identifier indexed_identifier;
            } lvalue;
            operator op;
            struct expression *rvalue;
        } classical_assignment;
    } classical;

    union
    {
        struct
        {
            struct list_head modifiers; /* Quantum Gate Modifier */
            identifier *name;
            struct list_head arguments; /* Expression */
            struct list_head qubits;    /* Qubit */
            struct expression *duration;
        } quantum_gate;

        struct
        {
            struct list_head modifiers; /* Quantum Gate Modifier */
            struct expression *argument;
            struct list_head qubits; /* Qubit */
        } quantum_phase;

        struct
        {
            struct list_head operands; /* Qubit */
        } quantum_nop;

        struct
        {
            quantum_measurement measure;
            qubit *target;
        } quantum_measurement;

        struct
        {
            struct list_head qubits; /* Expression */
        } quantum_barrier;

        struct
        {
            qubit *qubits;
        } quantum_reset;
    } qunatum;

} statement;

typedef struct expression
{
    expr_kind kind;

    struct expression *next;

    union
    {
        identifier *identifier;

        struct
        {
            operator op;
            struct expression *expr;
        } unary;

        struct
        {
            operator op;
            struct expression *lhs;
            struct expression *rhs;
        } binary;

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
                int b;
                char *bit_str;
                int timing;
                struct list_head array; /* Expression */
            } data;

        } literal;

        struct
        {
            identifier *name;
            struct list_head arguments; /* Expression */
        } function_call;

        struct
        {
            classical_type *type;
            struct expression *argument;
        } cast;

        struct
        {
            struct expression *collection;
            enum
            {
                DISCRETE_SET,
                EXPRESSION,
                RANGE
            } index_kind;
            union
            {
                discrete_set *discrete_set;
                struct list_head list; /* Union of expression and range definition */
            };

        } index;

        struct
        {
            struct expression *lhs;
            struct expression *rhs;
        } concat;
    } as;
} expression;

#endif
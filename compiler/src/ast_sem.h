#ifndef _AST_SEM_H_
#define _AST_SEM_H_

#include "ast_types.h"
#include "type.h"
#include "common.h"
#include "symrec.h"

#define INIT_LIST(_type) \
    _type *prev;         \
    _type *next;

#define DEFINE_WRAP_FUNC(type)                                 \
    static inline type##_list *wrap_##type##_list(type *value) \
    {                                                          \
        type##_list *_list = IALLOC(type##_list);              \
        _list->value = value;                                  \
        return _list;                                          \
    }

#define DEFINE_LIST(type)         \
    typedef struct type##_list    \
    {                             \
        struct type *value;       \
        struct type##_list *prev; \
        struct type##_list *next; \
    } type##_list;                \
    DEFINE_WRAP_FUNC(type)

#define list_add(type, new, head) \
    do                            \
    {                             \
        type *_new = new;         \
        type *_prev = head;       \
        _prev->next = _new;       \
        _new->prev = _prev;       \
        head = _new;              \
    } while (0)

#define list_for_each_entry(pos, head) \
    for (pos = head;                   \
         pos;                          \
         pos = pos->next)
#define list_reverse_for_each_entry(pos, head) \
    for (pos = head;                           \
         pos;                                  \
         pos = pos->prev)

#define list_add_all(type, list, head) \
    do                                 \
    {                                  \
        type *pos;                     \
        list_for_each_entry(pos, list) \
        {                              \
            list_add(type, pos, head); \
            head = pos;                \
        }                              \
    } while (0);

#define list_goto_first(type, list)              \
    do                                           \
    {                                            \
        type *_pos;                              \
        type *_head = list;                      \
        list_reverse_for_each_entry(_pos, _head) \
        {                                        \
            list = _pos;                         \
        }                                        \
    } while (0)

struct _sqz_program;

typedef enum
operator
{
    OP_DOUBLE_ASTERISK,
    OP_TILDE,
    OP_EXCLAMATION_POINT,
    OP_MINUS,
    OP_ASTERISK,
    OP_SLASH,
    OP_PERCENT,
    OP_PLUS,
    OP_LSHIFT,
    OP_RSHIFT,
    OP_LT,
    OP_GT,
    OP_GEQ,
    OP_LEQ,
    OP_EQ,
    OP_NEQ,
    OP_AMP,
    OP_PIPE,
    OP_CARET,
    OP_DOUBLE_AMP,
    OP_DOUBLE_PIPE,
    OP_ASSIGN,
    OP_MUL_ASSIGN,
    OP_DIV_ASSIGN,
    OP_MOD_ASSIGN,
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_LSHIFT_ASSIGN,
    OP_RSHIFT_ASSIGN,
    OP_AND_ASSIGN,
    OP_XOR_ASSIGN,
    OP_OR_ASSIGN
} operator;

typedef enum expr_kind
{
    EXPR_DISCRETE_SET,
    EXPR_EXPRESSION,
    EXPR_NONE,
    EXPR_RANGED_DEFINITION,
    EXPR_QUANTUM_MEASUREMENT,

    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_LITERAL,
    EXPR_FUNC_CALL,
    EXPR_CAST,
    EXPR_INDEX,
    EXPR_CONCAT,
    EXPR_IDENTIFIER
} expr_kind;

typedef enum stmt_kind
{
    STMT_ALIAS_DECLARATION,
    STMT_ASSIGNMENT,
    STMT_BARRIER,
    STMT_BOX,
    STMT_BREAK,
    STMT_CAL,
    STMT_CALIBRATION_GRAMMAR,
    STMT_CLASSICAL_DECLARATION,
    STMT_CONST_DECLARATION,
    STMT_CONTINUE,
    STMT_COMPOUND,
    STMT_DEF,
    STMT_DEFCAL,
    STMT_DELAY,
    STMT_END,
    STMT_EXPRESSION,
    STMT_EXTERN,
    STMT_FOR,
    STMT_GATE_CALL,
    STMT_GATE,
    STMT_IF,
    STMT_INCLUDE,
    STMT_IO_DECLARATION,
    STMT_MEASURE_ARROW_ASSIGNMENT,
    STMT_NOP,
    STMT_OLD_STYLE_DECLARATION,
    STMT_QUANTUM_DECLARATION,
    STMT_RESET,
    STMT_RETURN,
    STMT_SWITCH,
    STMT_WHILE
} stmt_kind;

typedef enum id_kind
{
    ID_INDEXED_IDENTIFIER,
    ID_IDENTIFIER
} id_kind;

typedef enum access_control
{
    READONLY,
    MUTABLE
} access_control;

typedef enum type_kind
{
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_COMPLEX,
    TYPE_ANGLE,
    TYPE_BIT,
    TYPE_BOOL,
    TYPE_ARRAY,
    TYPE_ARRAY_REF,
    TYPE_DURATION,
    TYPE_STRETCH,
    TYPE_QUBIT
} type_kind;

typedef struct symbol_t
{
    char *name;
} symbol_t;

typedef struct
{
    struct expression *size;
} int_type;

typedef struct
{
    struct expression *size;
} uint_type;

typedef struct
{
    struct expression *size;
} float_type;

typedef struct
{
    float_type *base_type;
} complex_type;

typedef struct
{
    struct expression *size;
} angle_type;

typedef struct
{
    struct expression *size;
} bit_type;

typedef struct
{
    struct expression *size;
} bool_type;

typedef struct duration_type
{

} duration_type;

typedef struct stetch_type
{

} stretch_type;

typedef enum
{
    SINGLE_EXPRESSION,
    EXPRESSION_LIST
} dimension_kind;

typedef struct
{
    type_kind kind;
    struct type *base_type;

    dimension_kind dimension_kind;

    union
    {
        struct expression *expr;
        struct expression_list *expr_list;
    } dimensions;

} array_type;

typedef struct
{
    type_kind kind;
    union
    {
        int_type *int_type;
        uint_type *uint_type;
        float_type *float_type;
        angle_type *angle_type;
        duration_type *duration_type;
        bit_type *bit_type;
        bool_type *bool_type;
        complex_type *complex_type;
    } base_type;

    dimension_kind dimension_kind;

    union
    {
        struct expression *expr;
        struct expression_list *expr_list;
    } dimensions;

} array_ref_type;

typedef struct qubit_type
{
    struct expression *size;
} qubit_type;

typedef struct type
{
    enum
    {
        CLASSICAL_TYPE,
        QUANTUM_TYPE
    } kind;

    union
    {
        struct classical_type *classical_type;
        struct quantum_type *quantum_type;
    };

} type;

typedef struct classical_type
{
    type_kind kind;

    char *type_name;
    union
    {
        int_type *int_type;
        uint_type *uint_type;
        float_type *float_type;
        angle_type *angle_type;
        duration_type *duration_type;
        bit_type *bit_type;
        bool_type *bool_type;
        complex_type *complex_type;
        array_type *array_type;
    };
} classical_type;

typedef struct quantum_type
{
    type_kind kind;
    char *type_name;
    union
    {
        qubit_type *qubit_type;
    };

} quantum_type;

typedef struct identifier
{
    char *name;
} identifier;

DEFINE_LIST(identifier);

typedef struct annotation
{
    char *keyword;
    char *command;
} annotation;

DEFINE_LIST(annotation);

typedef struct discrete_set
{
    struct expression *values;
} discrete_set;

typedef struct range_definition
{
    struct expression *start;
    struct expression *end;
    struct expression *step;
} range_definition;

typedef struct expr_or_range
{
    union
    {
        struct expression *expr;
        range_definition *range_definition;
    };
} expr_or_range;

DEFINE_LIST(expr_or_range);

typedef struct cls_args_or_expr
{
    union
    {
        struct classical_argument *argument;
        struct expression *expr;
    };
} cls_args_or_expr;

DEFINE_LIST(cls_args_or_expr);

typedef struct index_element
{
    expr_kind kind;
    union
    {
        discrete_set *discrete_set;
        expr_or_range *expr_or_range;
    } index;
} index_element;

DEFINE_LIST(index_element);

typedef struct indexed_identifier
{
    identifier *name;
    index_element *index;
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

DEFINE_LIST(quantum_gate_modifier);

typedef struct qubit
{
    id_kind kind;

    union
    {
        indexed_identifier *indexed_identifier;
        identifier *identifier;
    } value;

} qubit;

DEFINE_LIST(qubit);

typedef struct quantum_measurement
{
    qubit *qubit;
} quantum_measurement;

typedef struct quantum_argument
{
    identifier *name;
    struct expression *size;

} quantum_argument;

typedef struct extern_argument
{
    classical_type *type;
    access_control access;

} extern_argument;

DEFINE_LIST(extern_argument);

typedef struct classical_argument
{
    classical_type *type;
    identifier *name;
    access_control access;

} classical_argument;

DEFINE_LIST(classical_argument);

typedef struct cls_or_quantum_args
{
    enum arg_kind
    {
        CLASSICAL_ARGUMENT,
        QUANTUM_ARGUMENT
    } kind;
    union
    {
        classical_argument *classical_argument;
        quantum_argument *quantum_argument;
    };

} cls_or_quantum_args;

DEFINE_LIST(cls_or_quantum_args);

typedef struct case_stmt
{
    struct expression_list *expr;
    struct statement *statmenet;
} case_stmt;

DEFINE_LIST(case_stmt);

typedef struct statement
{
    stmt_kind kind;

    annotation_list *annotations;

    union
    {

        struct
        {
            struct statement_list *statements;
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
            identifier_list *identifiers;
            identifier_list *arguments;  /* Identifier */
            identifier_list *qubits;     /* Identifier */
            struct statement_list *body; /* Quantum Statement */
        } quantum_gate_definition;

        struct
        {
            identifier *name;
            extern_argument_list *arguments; /* Extern Argument */
            classical_type *return_type;
        } extern_declaration;

        struct
        {
            classical_type *type;
            identifier *identifier;
            expr_kind init_expression_kind;
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
            cls_args_or_expr_list *arguments;
            identifier_list *qubits;
            classical_type *return_type;
            char *body;
        } calibration_definition;

        struct
        {
            identifier *name;
            cls_or_quantum_args_list *arguments;
            struct statement_list *body;
            classical_type *return_type;
        } subroutine_definition;

        struct
        {
            expr_kind kind;
            union
            {
                struct expression *expr;
                quantum_measurement *measurement;
            } expr;
        } retrn;

        struct
        {
            struct expression *condition;
            struct statement_list *if_block;
            struct statement_list *else_block;
        } branching;

        struct
        {
            struct expression *condition;
            struct statement_list *block;
        } while_loop;

        struct
        {
            classical_type *type;
            identifier *identifier;
            expr_kind set_declaration_kind;
            union
            {
                range_definition *range_definition;
                discrete_set *discrete_set;
                struct expression *expression;
            } set_declaration;
            struct statement_list *block;
        } for_in_loop;

        struct
        {
            struct expression *target;
            case_stmt_list *cases;
            struct statement *deflt;
        } swtch;

        struct
        {
            struct expression *duration;
            qubit_list *qubits;
        } delay;

        struct
        {
            struct expression *duration;
            struct statement_list *body;
        } box;

        struct
        {
            struct statement_list *target;
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
            id_kind lvalue_kind;
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
            quantum_gate_modifier_list *modifiers; /* Quantum Gate Modifier */
            identifier *name;
            struct expression_list *arguments; /* Expression */
            qubit_list *qubits;                /* Qubit */
            struct expression *duration;
        } quantum_gate;

        struct
        {
            quantum_gate_modifier_list *modifiers; /* Quantum Gate Modifier */
            struct expression *argument;
            qubit_list *qubits; /* Qubit */
        } quantum_phase;

        struct
        {
            qubit_list *operands; /* Qubit */
        } quantum_nop;

        struct
        {
            quantum_measurement measure;
            qubit *target;
        } quantum_measurement;

        struct
        {
            struct expression_list *qubits; /* Expression */
        } quantum_barrier;

        struct
        {
            qubit *qubits;
        } quantum_reset;
    } quantum;

} statement;

DEFINE_LIST(statement);

typedef struct expression
{
    expr_kind kind;
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
                LIT_BIN_INT,
                LIT_OCT_INT,
                LIT_DEC_INT,
                LIT_HEX_INT,
                LIT_FLOAT,
                LIT_IMAGINARY,
                LIT_BOOL,
                LIT_BIT_STR,
                LIT_TIMING,
                LIT_HWQUBIT,
                LIT_IDENTIFIER
            } literal_kind;

            union
            {
                char *identifier;
                int i;
                float f;
                int b;
                char *bit_str;
                int timing;
                struct expression_list *array; /* Expression */
            } data;

        } literal;

        struct
        {
            identifier *name;
            struct expression_list *arguments; /* Expression */
        } function_call;

        struct
        {
            classical_type *type;
            struct expression *argument;
        } cast;

        struct
        {
            struct expression *collection;
            expr_kind index_kind;
            union
            {
                discrete_set *discrete_set;
                expr_or_range_list *list; /* Union of expression and range definition */
            };

        } index;

        struct
        {
            struct expression *lhs;
            struct expression *rhs;
        } concat;
    } as;
} expression;

DEFINE_LIST(expression);

typedef struct program
{
    statement_list *stmts;
} program;

void convert_program(const struct _sqz_program *p, program **out);

#endif
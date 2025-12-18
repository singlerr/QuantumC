#ifndef _AST_H_
#define _AST_H_

#include "symrec.h"
#include "type.h"

#ifndef AST_SIMPLE_NODE
#define AST_SIMPLE_NODE(node_type) new_ast_node(node_type, NULL, NULL, NULL, NULL, NULL, NULL)
#endif

#ifndef AST_GENERAL_NODE
#define AST_GENERAL_NODE(node_type, left, middle, right) new_ast_node(node_type, NULL, NULL, NULL, left, middle, right)
#endif

#ifndef AST_IDENTIFIER_NODE
#define AST_IDENTIFIER_NODE(node_type, id, left, middle, right) new_ast_node(node_type, id, NULL, NULL, left, middle, right)
#endif

#ifndef AST_TYPE_NODE
#define AST_TYPE_NODE(node_type, type, left, middle, right) new_ast_node(node_type, NULL, NULL, type, left, middle, right)
#endif

#ifndef AST_CONST_NODE
#define AST_CONST_NODE(node_type, constant) new_ast_node(node_type, NULL, constant, NULL, NULL, NULL, NULL)
#endif

#ifndef AST_ID_CURSCOPE
#define AST_ID_CURSCOPE(symbol, type) new_identifier_node(symbol, type, get_scope_level())
#endif

#ifndef AST_ID_NONSCOPE
#define AST_ID_NONSCOPE(symbol, type) new_identifier_node(symbol, type, -1)
#endif

typedef enum _ast_node_type
{
    AST_PROGRAM,
    AST_TRANSLATION_UNIT,
    AST_VARIABLE_DECLARATOR,
    AST_VARIABLE_DECLARATION,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_BODY,
    AST_PARAMETER_DECLARATION,
    AST_STRUCT_DECLARATION,
    AST_STRUCT_FIELD_DECLARATOR,
    AST_STRUCT_FIELD_DECLARATION,
    AST_ENUM_DECLARATION,
    AST_ENUM_FIELD_DECLARATION,
    AST_STRUCT,
    AST_UNION,
    AST_STRUCT_UNION,
    AST_ENUM,
    AST_NAME_TYPE,
    AST_ARRAY_ACCESS,
    AST_MEMBER_ACCESS,

    AST_NODE_LIST,

    AST_TYPE_INT,
    AST_TYPE_VOID,
    AST_TYPE_CHAR,
    AST_TYPE_SHORT,
    AST_TYPE_FLOAT,
    AST_TYPE_DOUBLE,
    AST_TYPE_SIGNED,
    AST_TYPE_UNSIGNED,
    AST_TYPE_LONG,
    AST_TYPE_BOOL,
    AST_TYPE_COMPLEX,
    AST_TYPE_IMAGINARY,
    AST_TYPE_POINTER,
    AST_TYPE_ARRAY,
    AST_TYPE_STRUCT,
    AST_TYPE_UNION,
    AST_TYPE_STRUCT_UNION,
    AST_TYPE_USER,
    AST_TYPE_FUNCTION,
    AST_TYPE_ENUM,

    AST_STMT_COMPOUND,
    AST_STMT_IF,
    AST_STMT_IF_ELSE,
    AST_STMT_SWITCH,
    AST_STMT_WHILE,
    AST_STMT_DO_WHILE,
    AST_STMT_FOR,
    AST_STMT_FOR_EXPR,
    AST_STMT_RETURN,
    AST_STMT_BREAK,
    AST_STMT_CONTINUE,
    AST_STMT_LABEL,
    AST_STMT_CASE,
    AST_STMT_DEFAULT,
    AST_STMT_GOTO,

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_ASSIGNMENT,
    AST_EXPR_ARRAY_ACCESS,
    AST_EXPR_POINTER_MEMBER_ACCESS,
    AST_EXPR_MEMBER_ACCESS,
    AST_EXPR_FUNCTION_CALL,
    AST_EXPR_POST_INC,
    AST_EXPR_POST_DEC,
    AST_EXPR_PRE_INC,
    AST_EXPR_PRE_DEC,
    AST_EXPR_TYPE_CAST,
    AST_EXPR_SIZEOF,

    AST_EXPR_MUL,
    AST_EXPR_ADD,
    AST_EXPR_SUB,
    AST_EXPR_DIV,
    AST_EXPR_MOD,

    AST_EXPR_LSHIFT,
    AST_EXPR_RSHIFT,

    AST_EXPR_LT,
    AST_EXPR_GT,
    AST_EXPR_GEQ,
    AST_EXPR_LEQ,

    AST_EXPR_EQ,
    AST_EXPR_NEQ,

    AST_EXPR_AND,
    AST_EXPR_OR,
    AST_EXPR_XOR,

    AST_EXPR_LAND,
    AST_EXPR_LOR,

    AST_EXPR_COND,

    AST_EXPR_ASSIGN,
    AST_EXPR_MUL_ASSIGN,
    AST_EXPR_DIV_ASSIGN,
    AST_EXPR_MOD_ASSIGN,
    AST_EXPR_ADD_ASSIGN,
    AST_EXPR_SUB_ASSIGN,
    AST_EXPR_LEFT_ASSIGN,
    AST_EXPR_RIGHT_ASSIGN,
    AST_EXPR_AND_ASSIGN,
    AST_EXPR_XOR_ASSIGN,
    AST_EXPR_OR_ASSIGN,

    AST_UNARY_AMP,
    AST_UNARY_STAR,
    AST_UNARY_PLUS,
    AST_UNARY_MINUS,
    AST_UNARY_TILDE,
    AST_UNARY_EXCL,

    AST_LITERAL_INTEGER,
    AST_LITERAL_STRING,
    AST_LITERAL_CHAR,
    AST_LITERAL_FLOAT,

    AST_IDENTIFIER,

    AST_STG_TYPEDEF,
    AST_STG_EXTERN,
    AST_STG_STATIC,
    AST_STG_AUTO,
    AST_STG_REGISTER,

    AST_QAL_CONST,
    AST_QAL_RESTRICT,
    AST_QAL_VOLATILE
} ast_node_type;

typedef struct _ast_identifier
{
    symrec_t *sym;
    type_t *type;
    int scope_level;
} ast_identifier_node;

typedef struct _ast_constant
{
    union data
    {
        int i;
        char c;
        float f;
        double d;
        char *s;
    } data;
} ast_const_node;

typedef struct _ast_node
{
    ast_node_type node_type;
    ast_identifier_node *identifier;
    ast_const_node *constant;
    typerec_t *type;
    struct _ast_node *left;
    struct _ast_node *right;
    struct _ast_node *middle;
} ast_node;

int get_scope_level();
void inc_scope_level();
void dec_scope_level();

ast_identifier_node *new_identifier_node(symrec_t *symbol, type_t *type, int scope_level);
ast_node *new_ast_node(ast_node_type node_type, const ast_identifier_node *id_node, const ast_const_node *const_node, const typerec_t *type, const ast_node *left, const ast_node *middle, const ast_node *right);
ast_const_node *new_ast_int_const(int i);
ast_const_node *new_ast_float_const(float f);
ast_const_node *new_ast_str_const(const char *s);

void append_left_child(ast_node *parent, const ast_node *child);
void append_right_child(ast_node *parent, const ast_node *child);
void append_middle_child(ast_node *parent, const ast_node *child);
const ast_node *find_last_left_child(const ast_node *parent);
const ast_node *find_last_right_child(const ast_node *parent);
const ast_node *find_last_middle_child(const ast_node *parent);
#endif
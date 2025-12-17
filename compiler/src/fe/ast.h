#ifndef _AST_H_
#define _AST_H_

#include "symrec.h"
#include "type.h"

#ifndef AST_SIMPLE_NODE
#define AST_SIMPLE_NODE(node_type) new_ast_node(node_type, NULL, NULL, NULL, NULL, NULL)
#endif

#ifndef AST_GENERAL_NODE
#define AST_GENERAL_NODE(node_type, left, middle, right) new_ast_node(node_type, NULL, NULL, left, middle, right)
#endif

#ifndef AST_IDENTIFIER_NODE
#define AST_IDENTIFIER_NODE(node_type, id, left, middle, right) new_ast_node(node_type, id, NULL, left, middle, right)
#endif

#ifndef AST_TYPE_NODE
#define AST_TYPE_NODE(node_type, type, left, middle, right) new_ast_node(node_type, NULL, type, left, middle, right)
#endif

#ifndef AST_ID_CURSCOPE
#define AST_ID_CURSCOPE(symbol, type) new_identifier_node(symbol, type, get_scope_level())
#endif

typedef enum _ast_node_type
{
    AST_PROGRAM,
    AST_TRANSLATION_UNIT,
    AST_VARIABLE_DECLARATOR,
    AST_VARIABLE_DECLARATION,
    AST_FUNCTION_DECLARATION,
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

    AST_STMT_COMPOUND,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_DO_WHILE,
    AST_STMT_FOR,
    AST_STMT_RETURN,
    AST_STMT_BREAK,
    AST_STMT_CONTINUE,

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_ASSIGNMENT,
    AST_EXPR_ARRAY_ACCESS,
    AST_EXPR_MEMBER_ACCESS,

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

typedef struct _ast_node
{
    ast_node_type node_type;
    ast_identifier_node *identifier;
    type_t *type;
    struct _ast_node *left;
    struct _ast_node *right;
    struct _ast_node *middle;
} ast_node;

int get_scope_level();
void inc_scope_level();
void dec_scope_level();

ast_identifier_node *new_identifier_node(symrec_t *symbol, type_t *type, int scope_level);
ast_node *new_ast_node(ast_node_type node_type, const ast_identifier_node *id_node, const type_t *type, const ast_node *left, const ast_node *middle, const ast_node *right);
void append_left_child(ast_node *parent, const ast_node *child);
void append_right_child(ast_node *parent, const ast_node *child);
void append_middle_child(ast_node *parent, const ast_node *child);
ast_node *find_last_left_child(ast_node *parent);
ast_node *find_last_right_child(ast_node *parent);
ast_node *find_last_middle_child(ast_node *parent);
#endif
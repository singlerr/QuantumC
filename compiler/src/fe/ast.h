#ifndef _AST_H_
#define _AST_H_

struct ast_node
{
    enum ast_node_type type;

    struct ast_node *left;
    struct ast_node *right;
};

struct ast_function_decl
{
    enum ast_node_type type;
};

struct ast_param_decl
{
    enum ast_node_type type;
    struct ast_node *params;
};

struct ast_struct_decl
{
    enum ast_node_type type;
};

struct ast_enum_decl
{
    enum ast_node_type type;
};

struct ast_data
{
    enum ast_node_type type;
    union
    {
        int i;
        char c;
        char *s;
        float f;
        void *p;
    } data;
};

struct ast_stmt_compound
{
    enum ast_node_type type;
    int count;
    struct ast_node *statements;
};

struct ast_stmt_if
{
    enum ast_node_type type;

    struct ast_node *true_part;
    struct ast_node *false_part;
};

struct ast_stmt_while
{
    enum ast_node_type type;

    struct ast_node *cond;
    struct ast_node *body;
};

struct ast_stmt_do_while
{
    enum ast_node_type type;
    struct ast_node *cond;
    struct ast_node *body;
};

struct ast_expr_binary
{
    enum ast_node_type type;

    int op;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_expr_unary
{
    enum ast_node_type type;
    int op;
    struct ast_node *target;
};

struct ast_expr_assignment
{
    enum ast_node_type type;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_expr_arr_access
{
    enum ast_node_type type;
    struct ast_node *array;
    struct ast_node *index;
};

struct ast_expr_member_access
{
    enum ast_node_type type;
    struct ast_node *owner;
    struct ast_node *member;
};

struct ast_identifier
{
    enum ast_node_type type;
    char *name;
};

enum ast_node_type
{
    AST_PROGRAM,
    AST_TRANSLATION_UNIT,
    AST_FUNCTION_DECLARATION,
    AST_PARAMETER_DECLARATION,
    AST_STRUCT_DECLARATION,
    AST_ENUM_DECLARATION,

    AST_TYPE_INT,
    AST_TYPE_CHAR,
    AST_TYPE_POINTER,
    AST_TYPE_ARRAY,
    AST_TYPE_STRUCT,

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

    AST_IDENTIFIER
};

struct ast_node *new_identifier(const char *identifier);
struct ast_node *new_ast_node_name(int code, const char *name, ...);
void append_child(struct ast_node *node, const struct ast_node *child);
const char *to_ast_string(int code);

#endif
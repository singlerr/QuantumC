#ifndef _AST_H_
#define _AST_H_

struct ast_node
{
    int code;
    int child_count;

    union
    {
        double d;
        float f;
        int i;
        unsigned int ui;
        int b;
        const char *str;
    } data;

    struct ast_node **children;
};

enum ast_node_type
{
    ABSTRACT_DECLARATOR,
    AND,
    ARG_LIST,
    ARRAY_DECLARATOR,
    ARRAY_DESIGNATOR,
    BITFIELD,
    BLOCK_ITEM_LIST,
    BREAK_STMT,
    CASE_STMT,
    CAST,
    COMMA_EXPR,
    COMPOUND_LITERAL,
    COMPOUND_STMT,
    CONDITIONAL,
    CONTINUE_STMT,
    DECLARATION,
    DECLARATION_LIST,
    DECLARATOR,
    DECL_SPECIFIERS,
    DEFAULT_STMT,
    DESIGNATED_INIT,
    DESIGNATOR_LIST,
    DIRECT_DECLARATOR,
    DO_WHILE_STMT,
    EMPTY,
    ENUMERATOR,
    ENUMERATOR_LIST,
    ENUM_SPECIFIER,
    EXCLUSIVE_OR,
    FOR_STMT,
    FUNCTION_DECLARATOR,
    FUNCTION_DEFINITION,
    GOTO_STMT,
    IDENTIFIER_LIST,
    IF_ELSE_STMT,
    IF_STMT,
    INCLUSIVE_OR,
    INIT_DECLARATOR,
    INIT_DECL_LIST,
    INIT_LIST,
    LABELED_STMT,
    MEMBER_DESIGNATOR,
    OP_ADDRESS_OF,
    OP_BITWISE_NOT,
    OP_DIVIDE,
    OP_GREATER_THAN,
    OP_LESS_THAN,
    OP_LOGICAL_NOT,
    OP_MINUS,
    OP_MODULO,
    OP_MULTIPLY,
    OP_PLUS,
    PARAM_DECLARATION,
    PARAM_LIST,
    POINTER,
    POSTFIX_ARRAY,
    POSTFIX_CALL,
    POSTFIX_DEC,
    POSTFIX_INC,
    POSTFIX_MEMBER,
    POSTFIX_PTR_MEMBER,
    PREFIX_DEC,
    PREFIX_INC,
    RETURN_STMT,
    SIZEOF_EXPR,
    SIZEOF_TYPE,
    SPEC_QUAL_LIST,
    STRUCT_DECLARATION,
    STRUCT_DECLARATOR_LIST,
    STRUCT_DECL_LIST,
    SWITCH_STMT,
    TRANSLATION_UNIT,
    TYPE_QUAL_LIST,
    WHILE_STMT
};

struct ast_node *new_ast_node(int code, ...);
struct ast_node *new_ast_node_name(int code, const char *name, ...);
void append_child(struct ast_node *node, const struct ast_node *child);
int to_ast_string(enum ast_node_type type, char *out_str);

#endif
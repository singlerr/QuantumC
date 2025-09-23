#include "ast.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

struct ast_node *new_ast_node(int code, ...)
{
    struct ast_node *temp;
    int i;
    int n = 0;
    va_list vl;
    va_start(vl, code);
    while (1)
    {
        temp = va_arg(vl, struct ast_node *);
        if (temp)
            n++;
        else
            break;
    }
    va_end(vl);

    struct ast_node *g = (struct ast_node *)malloc(sizeof(struct ast_node));
    g->children = 0;

    if (n > 0)
    {
        struct ast_node **children = (struct ast_node **)malloc(sizeof(struct ast_node *) * n);

        va_start(vl, code);
        i = 0;

        while (i < n)
        {
            temp = va_arg(vl, struct ast_node *);
            if (temp)
            {
                children[i++] = temp;
            }
        }

        g->children = children;
    }

    g->code = code;
    g->child_count = n;

    return g;
}

struct ast_node *new_ast_node_name(int code, const char *name, ...)
{

    struct ast_node *temp;
    int i;
    int n = 0;
    va_list vl;
    va_start(vl, name);
    while (1)
    {
        temp = va_arg(vl, struct ast_node *);
        if (temp)
            n++;
        else
            break;
    }
    va_end(vl);

    struct ast_node *g = (struct ast_node *)malloc(sizeof(struct ast_node));

    g->children = 0;

    if (n > 0)
    {
        struct ast_node **children = (struct ast_node **)malloc(sizeof(struct ast_node *) * n);

        va_start(vl, name);
        i = 0;

        while (i < n)
        {
            temp = va_arg(vl, struct ast_node *);
            if (temp)
            {
                children[i++] = temp;
            }
        }

        g->children = children;
        g->code = code;
    }

    g->child_count = n;

    g->data.str = strdup(name);
    return g;
}

int to_ast_string(enum ast_node_type type, char *out_str)
{
    switch (type)
    {
    case ABSTRACT_DECLARATOR:
        strcpy(out_str, "ABSTRACT_DECLARATOR");
        break;
    case AND:
        strcpy(out_str, "AND");
        break;
    case ARG_LIST:
        strcpy(out_str, "ARG_LIST");
        break;
    case ARRAY_DECLARATOR:
        strcpy(out_str, "ARRAY_DECLARATOR");
        break;
    case ARRAY_DESIGNATOR:
        strcpy(out_str, "ARRAY_DESIGNATOR");
        break;
    case BITFIELD:
        strcpy(out_str, "BITFIELD");
        break;
    case BLOCK_ITEM_LIST:
        strcpy(out_str, "BLOCK_ITEM_LIST");
        break;
    case BREAK_STMT:
        strcpy(out_str, "BREAK_STMT");
        break;
    case CASE_STMT:
        strcpy(out_str, "CASE_STMT");
        break;
    case CAST:
        strcpy(out_str, "CAST");
        break;
    case COMMA_EXPR:
        strcpy(out_str, "COMMA_EXPR");
        break;
    case COMPOUND_LITERAL:
        strcpy(out_str, "COMPOUND_LITERAL");
        break;
    case COMPOUND_STMT:
        strcpy(out_str, "COMPOUND_STMT");
        break;
    case CONDITIONAL:
        strcpy(out_str, "CONDITIONAL");
        break;
    case CONTINUE_STMT:
        strcpy(out_str, "CONTINUE_STMT");
        break;
    case DECLARATION:
        strcpy(out_str, "DECLARATION");
        break;
    case DECLARATION_LIST:
        strcpy(out_str, "DECLARATION_LIST");
        break;
    case DECLARATOR:
        strcpy(out_str, "DECLARATOR");
        break;
    case DECL_SPECIFIERS:
        strcpy(out_str, "DECL_SPECIFIERS");
        break;
    case DEFAULT_STMT:
        strcpy(out_str, "DEFAULT_STMT");
        break;
    case DESIGNATED_INIT:
        strcpy(out_str, "DESIGNATED_INIT");
        break;
    case DESIGNATOR_LIST:
        strcpy(out_str, "DESIGNATOR_LIST");
        break;
    case DIRECT_DECLARATOR:
        strcpy(out_str, "DIRECT_DECLARATOR");
        break;
    case DO_WHILE_STMT:
        strcpy(out_str, "DO_WHILE_STMT");
        break;
    case EMPTY:
        strcpy(out_str, "EMPTY");
        break;
    case ENUMERATOR:
        strcpy(out_str, "ENUMERATOR");
        break;
    case ENUMERATOR_LIST:
        strcpy(out_str, "ENUMERATOR_LIST");
        break;
    case ENUM_SPECIFIER:
        strcpy(out_str, "ENUM_SPECIFIER");
        break;
    case EXCLUSIVE_OR:
        strcpy(out_str, "EXCLUSIVE_OR");
        break;
    case FOR_STMT:
        strcpy(out_str, "FOR_STMT");
        break;
    case FUNCTION_DECLARATOR:
        strcpy(out_str, "FUNCTION_DECLARATOR");
        break;
    case FUNCTION_DEFINITION:
        strcpy(out_str, "FUNCTION_DEFINITION");
        break;
    case GOTO_STMT:
        strcpy(out_str, "GOTO_STMT");
        break;
    case IDENTIFIER_LIST:
        strcpy(out_str, "IDENTIFIER_LIST");
        break;
    case IF_ELSE_STMT:
        strcpy(out_str, "IF_ELSE_STMT");
        break;
    case IF_STMT:
        strcpy(out_str, "IF_STMT");
        break;
    case INCLUSIVE_OR:
        strcpy(out_str, "INCLUSIVE_OR");
        break;
    case INIT_DECLARATOR:
        strcpy(out_str, "INIT_DECLARATOR");
        break;
    case INIT_DECL_LIST:
        strcpy(out_str, "INIT_DECL_LIST");
        break;
    case INIT_LIST:
        strcpy(out_str, "INIT_LIST");
        break;
    case LABELED_STMT:
        strcpy(out_str, "LABELED_STMT");
        break;
    case MEMBER_DESIGNATOR:
        strcpy(out_str, "MEMBER_DESIGNATOR");
        break;
    case OP_ADDRESS_OF:
        strcpy(out_str, "OP_ADDRESS_OF");
        break;
    case OP_BITWISE_NOT:
        strcpy(out_str, "OP_BITWISE_NOT");
        break;
    case OP_DIVIDE:
        strcpy(out_str, "OP_DIVIDE");
        break;
    case OP_GREATER_THAN:
        strcpy(out_str, "OP_GREATER_THAN");
        break;
    case OP_LESS_THAN:
        strcpy(out_str, "OP_LESS_THAN");
        break;
    case OP_LOGICAL_NOT:
        strcpy(out_str, "OP_LOGICAL_NOT");
        break;
    case OP_MINUS:
        strcpy(out_str, "OP_MINUS");
        break;
    case OP_MODULO:
        strcpy(out_str, "OP_MODULO");
        break;
    case OP_MULTIPLY:
        strcpy(out_str, "OP_MULTIPLY");
        break;
    case OP_PLUS:
        strcpy(out_str, "OP_PLUS");
        break;
    case PARAM_DECLARATION:
        strcpy(out_str, "PARAM_DECLARATION");
        break;
    case PARAM_LIST:
        strcpy(out_str, "PARAM_LIST");
        break;
    case POINTER:
        strcpy(out_str, "POINTER");
        break;
    case POSTFIX_ARRAY:
        strcpy(out_str, "POSTFIX_ARRAY");
        break;
    case POSTFIX_CALL:
        strcpy(out_str, "POSTFIX_CALL");
        break;
    case POSTFIX_DEC:
        strcpy(out_str, "POSTFIX_DEC");
        break;
    case POSTFIX_INC:
        strcpy(out_str, "POSTFIX_INC");
        break;
    case POSTFIX_MEMBER:
        strcpy(out_str, "POSTFIX_MEMBER");
        break;
    case POSTFIX_PTR_MEMBER:
        strcpy(out_str, "POSTFIX_PTR_MEMBER");
        break;
    case PREFIX_DEC:
        strcpy(out_str, "PREFIX_DEC");
        break;
    case PREFIX_INC:
        strcpy(out_str, "PREFIX_INC");
        break;
    case RETURN_STMT:
        strcpy(out_str, "RETURN_STMT");
        break;
    case SIZEOF_EXPR:
        strcpy(out_str, "SIZEOF_EXPR");
        break;
    case SIZEOF_TYPE:
        strcpy(out_str, "SIZEOF_TYPE");
        break;
    case SPEC_QUAL_LIST:
        strcpy(out_str, "SPEC_QUAL_LIST");
        break;
    case STRUCT_DECLARATION:
        strcpy(out_str, "STRUCT_DECLARATION");
        break;
    case STRUCT_DECLARATOR_LIST:
        strcpy(out_str, "STRUCT_DECLARATOR_LIST");
        break;
    case STRUCT_DECL_LIST:
        strcpy(out_str, "STRUCT_DECL_LIST");
        break;
    case SWITCH_STMT:
        strcpy(out_str, "SWITCH_STMT");
        break;
    case TRANSLATION_UNIT:
        strcpy(out_str, "TRANSLATION_UNIT");
        break;
    case TYPE_QUAL_LIST:
        strcpy(out_str, "TYPE_QUAL_LIST");
        break;
    case WHILE_STMT:
        strcpy(out_str, "WHILE_STMT");
        break;
    default:
        strcpy(out_str, "UNKNOWN_TYPE");
        return -1; // 실패 반환
    }

    return 0; // 성공 반환
}
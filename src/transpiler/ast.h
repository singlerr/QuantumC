#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

struct type;

typedef struct var
{
  uint32_t id;
} var_t;

typedef struct typed_var
{
  var_t var;
  struct type type;
} typed_var_t;

typedef typed_var_t *typed_var_vec;

typedef struct strct
{
  typed_var_vec field;
} struct_t;

typedef struct qubit
{
  uint32_t size;
} qubit_t;

typedef struct angle
{
  uint32_t size;
  uint32_t value;
} angle_t;

typedef struct duration
{
  uint32_t value;
} duration_t;

typedef union literal
{
  int i;
  float f;
  short s;

} literal_t;

typedef struct stmt_compound
{
} stmt_compound_t;

typedef struct stmt_if
{

} stmt_if_t;

typedef struct stmt_if_else
{

} stmt_if_else_t;

typedef struct stmt_switch
{

} stmt_switch_t;

typedef struct stmt_while
{

} stmt_while_t;

typedef struct stmt_for
{

} stmt_for_t;

typedef struct node_id
{
  uint32_t id;
} node_id_t;

typedef enum ast_tag
{
  AST_VAR,
  AST_INT,
  AST_FLOAT,
  AST_SHORT,
  AST_QUBIT,
  AST_ANGLE,
  AST_DURATION,
  AST_FUN,
  AST_APP,
  AST_STRUCT,
  AST_UNION,
  AST_ENUM,

  AST_IF,
  AST_IF_ELSE,
  AST_SWITCH,
  AST_WHILE,
  AST_DO_WHILE,
  AST_FOR,
  AST_COMPOUND,

  AST_RETURN,
  AST_BREAK,
  AST_CONTINUE,
  AST_LABEL,
  AST_CASE,
  AST_DEFAULT,
  AST_GOTO,

  AST_ARRAY_ACCESS,
  AST_MEMBER_ACCESS,
  AST_POST_INC,
  AST_POST_DEC,
  AST_PRE_INC,
  AST_PRE_DEC,
  AST_CAST,
  AST_SIZEOF,

  AST_MUL,
  AST_DIV,
  AST_MOD,

  AST_ADD,
  AST_SUB,

  AST_LSHIFT,
  AST_RSHIFT,

  AST_LT,
  AST_GT,
  AST_GEQ,
  AST_LEQ,

  AST_EQ,
  AST_NEQ,

  AST_AND,
  AST_OR,
  AST_XOR,

  AST_LAND,
  AST_LOR,

  AST_ASSIGN,
  AST_ASSIGN_MUL,
  AST_ASSIGN_DIV,
  AST_ASSIGN_MOD,
  AST_ASSIGN_ADD,
  AST_ASSIGN_SUB,
  AST_ASSIGN_LSHIFT,
  AST_ASSIGN_RSHIFT,
  AST_ASSIGN_AND,
  AST_ASSIGN_OR,
  AST_ASSIGN_XOR
} ast_tag_t;

typedef struct ast
{
  ast_tag_t tag;
  node_id_t id;
  int lineno;
  union
  {

    var_t var;
    literal_t literal;
    qubit_t qubit;
    struct_t struct_t;
    struct_t union_t;
    angle_t angle;
    duration_t duration;
  };

} ast_t;

#endif
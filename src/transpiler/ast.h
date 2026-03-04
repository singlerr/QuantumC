#ifndef _AST_H_
#define _AST_H_

#include "data/vec/cvector.h"
#include "param.h"
#include "type/base.h"
#include <stdint.h>

typedef cvector_vector_type (struct typed_var) typed_var_vec;
typedef cvector_vector_type (struct struct_field) field_vec;
typedef cvector_vector_type (struct ast) ast_vec;
typedef cvector_vector_type (struct stmt_case) case_vec;

#define FOREACH_AST_TYPE(AST_TYPE)                                            \
  AST_TYPE (AST_VAR)                                                          \
  AST_TYPE (AST_INT)                                                          \
  AST_TYPE (AST_FLOAT)                                                        \
  AST_TYPE (AST_SHORT)                                                        \
  AST_TYPE (AST_QUBIT)                                                        \
  AST_TYPE (AST_ANGLE)                                                        \
  AST_TYPE (AST_DURATION)                                                     \
  AST_TYPE (AST_FUN)                                                          \
  AST_TYPE (AST_APP)                                                          \
  AST_TYPE (AST_STRUCT)                                                       \
  AST_TYPE (AST_UNION)                                                        \
  AST_TYPE (AST_ENUM)                                                         \
  AST_TYPE (AST_POINTER)                                                      \
  AST_TYPE (AST_IF)                                                           \
  AST_TYPE (AST_IF_ELSE)                                                      \
  AST_TYPE (AST_SWITCH)                                                       \
  AST_TYPE (AST_WHILE)                                                        \
  AST_TYPE (AST_DO_WHILE)                                                     \
  AST_TYPE (AST_FOR)                                                          \
  AST_TYPE (AST_COMPOUND)                                                     \
  AST_TYPE (AST_RETURN)                                                       \
  AST_TYPE (AST_BREAK)                                                        \
  AST_TYPE (AST_CONTINUE)                                                     \
  AST_TYPE (AST_LABEL)                                                        \
  AST_TYPE (AST_CASE)                                                         \
  AST_TYPE (AST_DEFAULT)                                                      \
  AST_TYPE (AST_GOTO)                                                         \
  AST_TYPE (AST_ARRAY_ACCESS)                                                 \
  AST_TYPE (AST_MEMBER_ACCESS)                                                \
  AST_TYPE (AST_POST_INC)                                                     \
  AST_TYPE (AST_POST_DEC)                                                     \
  AST_TYPE (AST_PRE_INC)                                                      \
  AST_TYPE (AST_PRE_DEC)                                                      \
  AST_TYPE (AST_CAST)                                                         \
  AST_TYPE (AST_SIZEOF)                                                       \
  AST_TYPE (AST_MUL)                                                          \
  AST_TYPE (AST_DIV)                                                          \
  AST_TYPE (AST_MOD)                                                          \
  AST_TYPE (AST_ADD)                                                          \
  AST_TYPE (AST_SUB)                                                          \
  AST_TYPE (AST_LSHIFT)                                                       \
  AST_TYPE (AST_RSHIFT)                                                       \
  AST_TYPE (AST_LT)                                                           \
  AST_TYPE (AST_GT)                                                           \
  AST_TYPE (AST_GEQ)                                                          \
  AST_TYPE (AST_LEQ)                                                          \
  AST_TYPE (AST_EQ)                                                           \
  AST_TYPE (AST_NEQ)                                                          \
  AST_TYPE (AST_AND)                                                          \
  AST_TYPE (AST_OR)                                                           \
  AST_TYPE (AST_XOR)                                                          \
  AST_TYPE (AST_LAND)                                                         \
  AST_TYPE (AST_LOR)                                                          \
  AST_TYPE (AST_ASSIGN)                                                       \
  AST_TYPE (AST_ASSIGN_MUL)                                                   \
  AST_TYPE (AST_ASSIGN_DIV)                                                   \
  AST_TYPE (AST_ASSIGN_MOD)                                                   \
  AST_TYPE (AST_ASSIGN_ADD)                                                   \
  AST_TYPE (AST_ASSIGN_SUB)                                                   \
  AST_TYPE (AST_ASSIGN_LSHIFT)                                                \
  AST_TYPE (AST_ASSIGN_RSHIFT)                                                \
  AST_TYPE (AST_ASSIGN_AND)                                                   \
  AST_TYPE (AST_ASSIGN_OR)                                                    \
  AST_TYPE (AST_ASSIGN_XOR)

typedef struct ast_fun
{
  type_t *ty_fun;
  ast_t *body;
} ast_fun_t;

typedef struct pointer
{
  int constraints;
  ast_t *ref;
} pointer_t;

typedef struct struct_field
{
  typed_var_t var;
} struct_field_t;

typedef struct var
{
  char name[SYM_MAXLEN];
  uint32_t id;
} var_t;

typedef struct app
{
  struct ast *fun;
  ast_vec args;
} app_t;

typedef struct array_access
{
  struct ast *array;
  struct ast *index;
} array_access_t;

typedef struct member_access
{
  struct ast *aggregate;
  struct ast *member;
} member_access_t;

typedef struct typed_var
{
  var_t var;
  struct type type;
} typed_var_t;

typedef struct strct
{
  field_vec field;
} struct_t;

typedef struct qubit
{
} qubit_t;

typedef struct angle
{
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
  int b;
  duration_t duration;
} literal_t;

typedef struct stmt_compound
{
  ast_vec ast;
} stmt_compound_t;

typedef struct stmt_if
{
  struct ast *condition;
  struct ast *body;
} stmt_if_t;

typedef struct stmt_if_else
{
  struct ast *condition;
  struct ast *body;
  struct ast *else_body;
} stmt_if_else_t;

typedef struct stmt_case
{
  struct ast *body;
  struct ast *constant;
} stmt_case_t;

typedef struct stmt_switch
{
  struct ast *expr;
  case_vec body;
} stmt_switch_t;

typedef struct stmt_while
{
  struct ast *condition;
  struct ast *body;
} stmt_while_t;

typedef struct stmt_for
{
  struct ast *lhs;
  struct ast *mhs;
  struct ast *rhs;
  struct ast *body;
} stmt_for_t;

typedef struct node_id
{
  uint32_t id;
} node_id_t;

typedef struct expr_binary
{
  struct ast *lhs;
  struct ast *rhs;
} expr_binary_t;

// FEAT:
// virtualize array access as function application
// e.g. array_access(arr, index)
typedef struct expr_unary
{
  struct ast *value;
} expr_unary_t;

typedef struct expr_ternary
{
  struct ast *lhs;
  struct ast *mhs;
  struct ast *rhs;
} expr_ternary_t;

#define ENUM_GEN(ENUM) ENUM,

typedef enum ast_tag
{
  FOREACH_AST_TYPE (ENUM_GEN) AST_TYPE_MAX
} ast_tag_t;

#undef ENUM_GEN

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
    app_t app;
    ast_fun_t fun;
    array_access_t arr_access;
    member_access_t member_access;
    pointer_t pointer;

    stmt_compound_t stmt_compound;
    stmt_if_t stmt_if;
    stmt_if_else_t stmt_if_else;
    stmt_switch_t stmt_switch;
    stmt_while_t stmt_while;
    stmt_for_t stmt_for;
    expr_unary_t expr_unary;
    expr_binary_t expr_binary;
    expr_ternary_t expr_ternary;
  };

} ast_t;

node_id_t new_node_id (uint32_t id);
literal_t new_literal_int (int i);
literal_t new_literal_float (float f);
literal_t new_literal_short (short s);
literal_t new_literal_bool (int b);
qubit_t new_qubit (uint32_t size);
angle_t new_angle (uint32_t size, uint32_t value);
duration_t new_duration (uint32_t value);
var_t new_var (uint32_t id, const char *name);
ast_fun_t new_fun (var_t arg, struct ast *body);
app_t new_app (struct ast *fun, struct ast *arg);
array_access_t new_arr_access (struct ast *array, struct ast *index);
member_access_t new_member_access (struct ast *aggregate, struct ast *member);
expr_unary_t new_unary_expr (ast_t *value);
pointer_t new_pointer (ast_t *ref, int constr);

ast_t *new_ast ();
void init_ast_ctx ();
node_id_t next_id ();
uint32_t next_var_id ();
const char *to_ast_string (ast_tag_t tag);

#define Int(i) new_literal_int (i)
#define Float(f) new_literal_float (f)
#define Bool(b) new_literal_bool (b)
#define Var(name) new_var (next_var_id (), name)
#define Fun(arg, body) new_fun (arg, body)
#define App(fun, arg) new_app (fun, arg)
#define ArrAccess(array, index) new_arr_access (array, index)
#define MemAccess(aggregate, member) new_member_access (aggregate, member)
#define Id(i) new_node_id (i)
#define Unary(ast) new_unary_expr (ast)
#define Pointer(ref, constr) new_pointer (ref, constr)
#define AST_NAME(ast_tag) to_ast_string (ast_tag)

#endif
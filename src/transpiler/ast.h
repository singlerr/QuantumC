#ifndef _AST_H_
#define _AST_H_

#include "ast_common_types.h"
#include "symrec.h"
#include "type.h"
#include <stdint.h>

#ifndef AST_SIMPLE_NODE
#define AST_SIMPLE_NODE(node_type)                                            \
  new_ast_node (node_type, NULL, NULL, NULL, NULL, NULL, NULL)
#endif

#ifndef AST_GENERAL_NODE
#define AST_GENERAL_NODE(node_type, left, middle, right)                      \
  new_ast_node (node_type, NULL, NULL, NULL, left, middle, right)
#endif

#ifndef AST_IDENTIFIER_NODE
#define AST_IDENTIFIER_NODE(node_type, id, left, middle, right)               \
  new_ast_node (node_type, id, NULL, NULL, left, middle, right)
#endif

#ifndef AST_TYPE_NODE
#define AST_TYPE_NODE(node_type, type, left, middle, right)                   \
  new_ast_node (node_type, NULL, NULL, type, left, middle, right)
#endif

#ifndef AST_CONST_NODE
#define AST_CONST_NODE(node_type, constant)                                   \
  new_ast_node (node_type, NULL, constant, NULL, NULL, NULL, NULL)
#endif

#ifndef AST_ID_CURSCOPE
#define AST_ID_CURSCOPE(symbol, type)                                         \
  new_identifier_node (symbol, type, get_scope_level ())
#endif

#ifndef AST_ID_NONSCOPE
#define AST_ID_NONSCOPE(symbol, type) new_identifier_node (symbol, type, -1)
#endif

typedef struct node_id
{
  uint32_t id;
} node_id_t;

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

int get_scope_level ();
void inc_scope_level ();
void dec_scope_level ();

ast_identifier_node *new_identifier_node (symrec_t *symbol, type_t *type,
                                          int scope_level);
ast_node *new_ast_node (ast_node_type node_type,
                        const ast_identifier_node *id_node,
                        const ast_const_node *const_node,
                        const typerec_t *type, const ast_node *left,
                        const ast_node *middle, const ast_node *right);
ast_const_node *new_ast_int_const (int i);
ast_const_node *new_ast_float_const (float f);
ast_const_node *new_ast_str_const (const char *s);
ast_const_node *new_ast_bool_const (int b);
int register_type_if_required (ast_node *decl, ast_node *identifier);

void append_left_child (ast_node *parent, const ast_node *child);
void append_right_child (ast_node *parent, const ast_node *child);
void append_middle_child (ast_node *parent, const ast_node *child);
const ast_node *find_last_left_child (const ast_node *parent);
const ast_node *find_last_right_child (const ast_node *parent);
const ast_node *find_last_middle_child (const ast_node *parent);
#endif
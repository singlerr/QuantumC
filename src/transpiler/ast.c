#include "ast.h"
#include "common.h"
#include "param.h"
#include <string.h>

#define STRING_GEN(STRING) #STRING,
const char *ast_to_str[] = { FOREACH_AST_TYPE (STRING_GEN) };
#undef STRING_GEN

static struct
{
  uint32_t node_id;
} ast_ctx = { .node_id = 0 };

static uint32_t var_id_counter = 0;

ast_t *
new_ast ()
{
  ast_t *ast = IALLOC (ast_t);
  ast->id = next_id ();
  return ast;
}

void
init_ast_ctx ()
{
  ast_ctx.node_id = 0;
  var_id_counter = 0;
}

node_id_t
next_id ()
{
  return Id (ast_ctx.node_id++);
}

uint32_t
next_var_id ()
{
  return ++var_id_counter;
}

node_id_t
new_node_id (uint32_t id)
{
  return (node_id_t){ .id = id };
}

const char *
to_ast_string (ast_tag_t tag)
{
  return ast_to_str[tag];
}

literal_t
new_literal_int (int i)
{
  literal_t l;
  memset (&l, 0, sizeof (l));
  l.i = i;
  return l;
}

literal_t
new_literal_float (float f)
{
  literal_t l;
  memset (&l, 0, sizeof (l));
  l.f = f;
  return l;
}

literal_t
new_literal_short (short s)
{
  literal_t l;
  memset (&l, 0, sizeof (l));
  l.s = s;
  return l;
}

literal_t
new_literal_bool (int b)
{
  literal_t l;
  memset (&l, 0, sizeof (l));
  l.b = b;
  return l;
}

var_t
new_var (uint32_t id, const char *name)
{
  var_t v;
  v.id = id;
  if (name)
    strncpy (v.name, name, SYM_MAXLEN);
  else
    v.name[0] = '\0';
  return v;
}

qubit_t
new_qubit (uint32_t size)
{
  (void)size;
  return (qubit_t){};
}

angle_t
new_angle (uint32_t size, uint32_t value)
{
  (void)size;
  return (angle_t){ .value = value };
}

duration_t
new_duration (uint32_t value)
{
  return (duration_t){ .value = value };
}

ast_fun_t
new_fun (var_t arg, struct ast *body)
{
  (void)arg;
  return (ast_fun_t){ .body = body };
}

app_t
new_app (struct ast *fun, struct ast *arg)
{
  app_t a;
  a.fun = fun;
  a.args = NULL;
  if (arg)
    cvector_push_back (a.args, arg);
  return a;
}

array_access_t
new_arr_access (struct ast *array, struct ast *index)
{
  return (array_access_t){ .array = array, .index = index };
}

member_access_t
new_member_access (struct ast *aggregate, struct ast *member)
{
  return (member_access_t){ .aggregate = aggregate, .member = member };
}

expr_unary_t
new_unary_expr (ast_t *value)
{
  return (expr_unary_t){ .value = value };
}

expr_binary_t
new_binary_expr (ast_t *lhs, ast_t *rhs)
{
  return (expr_binary_t){ .lhs = lhs, .rhs = rhs };
}

expr_ternary_t
new_ternary_expr (ast_t *lhs, ast_t *mhs, ast_t *rhs)
{
  return (expr_ternary_t){ .lhs = lhs, .mhs = mhs, .rhs = rhs };
}

expr_cast_t
new_cast_expr (ty_deco_t *ty, ast_t *value)
{
  return (expr_cast_t){ .ty_caster = ty, .value = value };
}

expr_list_t
new_expr_list (ast_t *prev, ast_t *value, ty_deco_t *ty)
{
  (void)ty;
  return (expr_list_t){ .prev = prev, .value = value };
}

int
is_assignment_operator (ast_tag_t tag)
{
  return tag == AST_ASSIGN || tag == AST_ASSIGN_MUL || tag == AST_ASSIGN_DIV
         || tag == AST_ASSIGN_MOD || tag == AST_ASSIGN_ADD
         || tag == AST_ASSIGN_SUB || tag == AST_ASSIGN_LSHIFT
         || tag == AST_ASSIGN_RSHIFT || tag == AST_ASSIGN_AND
         || tag == AST_ASSIGN_OR || tag == AST_ASSIGN_XOR;
}

int
is_binary_operator (ast_tag_t tag)
{
  return tag == AST_MUL || tag == AST_DIV || tag == AST_MOD || tag == AST_ADD
         || tag == AST_SUB || tag == AST_LSHIFT || tag == AST_RSHIFT
         || tag == AST_LT || tag == AST_GT || tag == AST_LEQ || tag == AST_GEQ
         || tag == AST_EQ || tag == AST_NEQ || tag == AST_AND || tag == AST_OR
         || tag == AST_XOR || tag == AST_LAND || tag == AST_LOR;
}

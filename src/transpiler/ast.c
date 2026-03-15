#include "ast.h"
#include "common.h"

#define STRING_GEN(STRING) #STRING,
const char *ast_to_str[] = { FOREACH_AST_TYPE (STRING_GEN) };
#undef STRING_GEN

struct
{
  uint32_t node_id;
} ast_ctx = { .node_id = 0 };

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
}

node_id_t
next_id ()
{
  return Id (ast_ctx++);
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

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
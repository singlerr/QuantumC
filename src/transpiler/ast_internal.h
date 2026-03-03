#ifndef _AST_INTERNAL_H_
#define _AST_INTERNAL_H_

#include "ast.h"
#include "common.h"
#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>

static inline var_t
search_var (const char *name)
{
  const symtab_t *s = search_symbol (name, FALSE);
  if (!s)
    {
      // TODO: How to handle var id
      return Var (-1);
    }

  return Var (s->id);
}

static inline ast_t *
new_ast_var (var_t var)
{
  ast_t *ast = new_ast ();
  ast->var = var;
  return ast;
}

static inline ast_t *
new_ast_literal (ast_tag_t type, literal_t literal)
{
  ast_t *ast = new_ast ();
  ast->literal = literal;
  return ast;
}

static inline ast_t *
new_ast_arr_access (array_access_t arr_access)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_ARRAY_ACCESS;
  ast->arr_access = arr_access;
  return ast;
}

static inline ast_t *
new_ast_member_access (member_access_t member_access)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_MEMBER_ACCESS;
  ast->member_access = member_access;
  return ast;
}

static inline ast_t *
new_ast_app (app_t app)
{
  ast_t *ast = new_ast ();
  ast->app = app;
  return ast;
}

static inline ast_t *
new_ast_unary_expr (ast_tag_t type, expr_unary_t unary)
{
  ast_t *ast = new_ast ();
  ast->tag = type;
  ast->expr_unary = unary;
  return ast;
}

static inline ast_t *
new_ast_pointer (pointer_t ptr)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_POINTER;
  ast->pointer = ptr;
  return ast;
}

static inline const struct_field_t *
search_struct_member (ast_t *strct, const char *name)
{
  struct_field_t *it;
  if (strct->tag != AST_STRUCT)
    {
      return NULL;
    }

  for (it = cvector_begin (strct->struct_t.field);
       it != cvector_end (strct->struct_t.field); it++)
    {
      if (!strcmp (it->var.var.name, name))
        {
          return it;
        }
    }

  return NULL;
}

static inline ast_t *
from_struct_member (ast_t *strct, const char *name)
{
  if (!strct)
    {
      warn ("Struct is null");
      return NULL;
    }
  struct_field_t *field = search_struct_member (strct, name);
  if (!field)
    {
      warn ("%s is not a member of struct", name);
      return NULL;
    }

  return new_ast_var (field->var.var);
}

static inline ast_t *
ref_pointer (ast_t *ptr)
{
  if (ptr->tag != AST_POINTER)
    {
      warn ("Expected pointer but found");
      return NULL;
    }

  return ptr->pointer.ref;
}

static inline pointer_t *
find_tail_pointer (pointer_t *ptr)
{
  while (1)
    {
      if (!ptr->ref)
        {
          return ptr;
        }

      if (ptr->ref->tag != AST_POINTER)
        {
          return ptr;
        }

      ptr = &ptr->ref->pointer;
    }

  return NULL;
}

#endif
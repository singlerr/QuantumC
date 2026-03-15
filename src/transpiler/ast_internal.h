#ifndef _AST_INTERNAL_H_
#define _AST_INTERNAL_H_

#include "ast.h"
#include "common.h"
#include "err.h"
#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
new_ast_var (var_t var, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->var = var;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_literal (ast_tag_t type, literal_t literal, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->literal = literal;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_arr_access (array_access_t arr_access, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_ARRAY_ACCESS;
  ast->arr_access = arr_access;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_member_access (member_access_t member_access, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_MEMBER_ACCESS;
  ast->member_access = member_access;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_app (app_t app, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->app = app;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_unary_expr (ast_tag_t type, expr_unary_t unary, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->tag = type;
  ast->expr_unary = unary;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_binary_expr (ast_tag_t tag, expr_binary_t binary, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->ty = ty;
  ast->expr_binary = binary;
  ast->tag = tag;
  return ast;
}

static inline ast_t *
new_ast_ternary_expr (ast_tag_t tag, expr_ternary_t ternary, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->tag = tag;
  ast->expr_ternary = ternary;
  ast->ty = ty;
  return ast;
}

static inline ast_t *
new_ast_cast_expr (expr_cast_t cast)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_CAST;
  ast->expr_cast = cast;
  ast->ty = cast.ty_caster;
  return ast;
}

static inline ast_t *
new_ast_expr_list (expr_list_t list, ty_deco_t *ty)
{
  ast_t *ast = new_ast ();
  ast->tag = AST_LIST;
  ast->expr_list = list;
  ast->ty = ty;
  return ast;
}

static inline const struct_field_t *
search_struct_member (type_t *strct, const char *name)
{
  struct_field_t *it;
  if (strct->tag != TY_STRUCT)
    {
      return NULL;
    }

  for (it = cvector_begin (strct->ty.ty_struct.fields);
       it != cvector_end (strct->ty.ty_struct.fields); it++)
    {
      if (!strcmp (it->var.var.name, name))
        {
          return it;
        }
    }

  return NULL;
}

static inline ast_t *
from_struct_member (ty_deco_t *strct, const char *name)
{
  if (!strct)
    {
      warn ("Struct type is null");
      return NULL;
    }
  struct_field_t *field = search_struct_member (strct->ty, name);
  if (!field)
    {
      warn ("%s is not a member of struct", name);
      return NULL;
    }

  return new_ast_var (field->var.var);
}

static inline ty_pointer_t *
pointer_tail (ty_pointer_t *ptr)
{
  for (; ptr->ref; ptr = &ptr->ref->ty->ty.ty_pointer)
    ;

  return ptr;
}

static inline ty_deco_t *
ref_pointer (ty_deco_t *ty)
{
  if (ty->ty->tag != TY_POINTER)
    {

      warn ("Could not ref non-pointer type");
      return NULL;
    }

  return ty->ty->ty.ty_pointer.ref;
}

static inline decl_t *
begin_decl (const char *name)
{
  decl_t *decl = IALLOC (decl_t);
  decl->has_name = TRUE;
  strncpy (decl->name, name, SYM_MAXLEN);
  return decl;
}

static inline decl_t *
decl_array (decl_t *decl, int size)
{
  ty_deco_t *deco = new_ty_array (
      (ty_array_t){ .ref = decl->type, .size = size }, CONSTR_EMPTY);
  decl->type = deco;
  return decl;
}

static int
ast_sizeof (ast_t *expr)
{
  if (!expr)
    {
      warn ("Sizeof argument must be not null");
      return -1;
    }

  switch (expr->tag)
    {
    case AST_VAR:
      {
        const ty_deco_t *ty = search_symbol_type (expr->var.name);
        if (!ty)
          {
            warn ("Variable \"%s\" does not exist.", expr->var.name);
            return -1;
          }

        return type_sizeof (ty);
      }
      break;
    default:
      return type_sizeof (expr->ty);
    }
}

static decl_list_t
deco_init_declarator (decl_list_t list, ty_deco_t *type)
{
  decl_t *it;
  ty_deco_t *ty;

  for (it = cvector_begin (list); it != cvector_end (list); it++)
    {
      ty = clone_ty_deco (type);
      if (it->type)
        {
          // connect type
          ty = append_ty (it->type, ty);
          it->type = ty;
        }
      else
        {
          it->type = ty;
        }
    }

  return list;
}

#define CALC_OP(operator, lhs, rhs, result)                                   \
  do                                                                          \
    {
if (lhs.type == AST_INT)
}
while (0)
  ;

static const_result_t
join_const (ast_tag_t op, const_result_t lhs, const_result_t rhs)
{
  const_result_t result;
  switch (op)
    {
    case AST_ADD:

      break;
    case AST_SUB:
      break;
    case AST_MUL:
      break;
    case AST_DIV:
      break;
    case AST_MOD:
      break;
    case AST_GT:
      break;
    case AST_LT:
      break;
    case AST_LEQ:
      break;
    case AST_GEQ:
      break;
    case AST_LSHIFT:
      break;
    case AST_RSHIFT:
      break;
    case AST_EQ:
      break;
    case AST_NEQ:
      break;
    case AST_AND:
      break;
    case AST_OR:
      break;
    case AST_XOR:
      break;
    case AST_LAND:
      break;
    case AST_LOR:
      break;
    case AST_ASSIGN_MUL:
      break;
    case AST_ASSIGN_DIV:
      break;
    case AST_ASSIGN_MOD:
      break;
    case AST_ASSIGN_ADD:
      break;
    case AST_ASSIGN_SUB:
      break;
    case AST_ASSIGN_LSHIFT:
      break;
    case AST_ASSIGN_RSHIFT:
      break;
    case AST_ASSIGN_AND:
      break;
    case AST_ASSIGN_OR:
      break;
    case AST_ASSIGN_XOR:
      break;
    default:
      break;
    }
}

static const_result_t
fold_assignment_expr (ast_t *ast)
{
  ast_tag_t tag = ast->tag;
  const_result_t lhs, rhs;
  const_result_t result;
  switch (tag)
    {
    case AST_INT:
      result.type = AST_INT;
      result.value.i = ast->literal.i;
      return result;
    case AST_FLOAT:
      result.type = AST_FLOAT;
      result.value.f = ast->literal.f;
      return result;
    case AST_SHORT:
      result.type = AST_SHORT;
      result.value.s = ast->literal.s;
      return result;
    default:
      break;
    }

  if (is_assignment_operator (ast->tag))
    {
      lhs = fold_assignment_expr (ast->expr_binary.lhs);
      rhs = fold_assignment_expr (ast->expr_binary.rhs);
    }
}

#endif
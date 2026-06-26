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
  return new_var (next_var_id (), name);
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

static inline struct_field_t *
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
      if (!strcmp (it->name, name))
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

  return new_ast_var (Var (field->name), field->ty);
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
        ty_deco_t *ty = search_symbol_type (expr->var.name, FALSE);
        if (!ty)
          {
            warn ("Variable \"%s\" does not exist.", expr->var.name);
            return -1;
          }

        return type_sizeof (ty->ty);
      }
      break;
    default:
      return expr->ty ? type_sizeof (expr->ty->ty) : -1;
    }
}

static decl_list_t
deco_init_declarator (decl_list_t list, ty_deco_t *type)
{
  decl_t *it;
  ty_deco_t *ty;
  int is_typedef = type && (type->constr & CONSTR_TYPEDEF);

  for (it = cvector_begin (list); it != cvector_end (list); it++)
    {
      ty = clone_ty_deco (type);
      if (it->type)
        ty = append_ty (it->type, ty);
      it->type = ty;

      if (!it->has_name)
        continue;
      if (is_typedef)
        push_type (it->name, ty ? ty->ty : NULL);
      else
        put_symbol (it->name, ty);
    }

  return list;
}


#endif
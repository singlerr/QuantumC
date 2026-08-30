#ifndef _HIR_H_
#define _HIR_H_

#include <stdint.h>

#include "../ast_sem.h"
#include "../data/vec/cvector.h"
#include "../type/base.h"

typedef enum _h_opcode
{
  H_COPY,
  H_ADD,
  H_SUB,
  H_MUL,
  H_DIV,
  H_MOD,
  H_REM,
  H_POW,
  H_SHL,
  H_SHR,
  H_AND,
  H_OR,
  H_XOR,
  H_NOT,
  H_NEG,
  H_COMP,
  H_INC,
  H_DEC,
  H_LT,
  H_LE,
  H_EQ,
  H_NE,
  H_GE,
  H_GT,
  H_ADDR,
  H_GET,
  H_SET,

  H_LABEL,
  H_JUMP,
  H_CJUMP,
  H_CALL,
  H_PARAM,
  H_RETURN,
} h_opcode;

typedef enum _h_lit_type
{
  H_INT,
  H_FLOAT
} h_lit_type;

typedef struct _hir_var
{
  char *name;
  uint32_t owner;
  type_t *ty;
} hir_var;

typedef struct _hir_temp
{
  uint32_t id;
  type_t *ty;
} hir_temp;

typedef struct _hir_lit
{
  h_lit_type type;
  uint32_t size;
  union
  {
    int i;
    float f;
  } val;
} hir_lit;

typedef struct _hir_label
{
  char *name;
} hir_label;

typedef enum _h_operand_kind
{
  HO_NONE,
  HO_VAR,
  HO_TEMP,
  HO_LIT,
  HO_LABEL
} h_operand_kind;

typedef struct _hir_operand
{
  h_operand_kind kind;
  union
  {
    hir_var var;
    hir_temp tmp;
    hir_lit lit;
    hir_label label;
  };
} hir_operand;

struct _hir;

typedef cvector_vector_type (hir_var) hir_var_vec;

typedef struct _hir_sub
{
  char *name;
  type_t *ty_fun;
  hir_var_vec params;
  hir_var_vec locals;
  struct _hir *entry;
} hir_sub;

typedef struct _hir
{
  h_opcode op;
  hir_operand dst;
  hir_operand src1;
  hir_operand src2;

  struct _hir *prev;
  struct _hir *next;
} hir;

typedef cvector_vector_type (hir_sub) hir_sub_vec;

typedef struct _hir_program
{
  hir_var_vec globals;
  hir_sub_vec subs;
} hir_program;

void hir_from_program (program *prog, hir_program *out);
void hir_cleanup (hir **h);

#endif
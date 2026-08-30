#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "diagnostics.h"
#include "type/base.h"

FILE *fout = NULL;
int indent = 0;

void gen (const char *msg, ...);
void gen_indent (void);
void newline (void);
void space (void);
void begin_bracket (void);
void end_bracket (void);

static void gen_operand (const hir_operand *o);
static void gen_instr (const hir *ins);
static void gen_sub (const hir_sub *s);
static void gen_var_decl (const hir_var *v);

void
set_codegen_output (FILE *out)
{
  fout = out;
}

void
gen_program (hir_program *prog)
{
  hir_var *g;
  for (g = cvector_begin (prog->globals); g != cvector_end (prog->globals); g++)
    {
      gen_var_decl (g);
      newline ();
    }

  size_t i;
  for (i = 0; i < cvector_size (prog->subs); i++)
    {
      if (!cvector_empty (prog->globals) || i > 0)
        newline ();
      gen_sub (&prog->subs[i]);
    }
}

static void
gen_var_decl (const hir_var *v)
{
  gen ("global %s %s;", v->ty ? type_to_str (v->ty->tag) : "int", v->name);
}

static void
gen_sub (const hir_sub *s)
{
  gen ("sub %s:", s->name);
  newline ();

  indent++;
  const hir *ins;
  for (ins = s->entry; ins; ins = ins->next)
    {
      gen_instr (ins);
      newline ();
    }
  indent--;
}

static void
gen_operand (const hir_operand *o)
{
  switch (o->kind)
    {
    case HO_VAR:
      gen ("%s", o->var.name);
      break;
    case HO_TEMP:
      gen ("t%u", o->tmp.id);
      break;
    case HO_LIT:
      if (o->lit.type == H_INT)
        gen ("%d", o->lit.val.i);
      else
        gen ("%f", o->lit.val.f);
      break;
    case HO_LABEL:
      gen ("%s", o->label.name);
      break;
    case HO_NONE:
      break;
    }
}

static const char *
h_binop_symbol (h_opcode op)
{
  switch (op)
    {
    case H_ADD: return "+";
    case H_SUB: return "-";
    case H_MUL: return "*";
    case H_DIV: return "/";
    case H_MOD: return "%";
    case H_REM: return "rem";
    case H_POW: return "**";
    case H_SHL: return "<<";
    case H_SHR: return ">>";
    case H_AND: return "&";
    case H_OR: return "|";
    case H_XOR: return "^";
    case H_LT: return "<";
    case H_LE: return "<=";
    case H_EQ: return "==";
    case H_NE: return "!=";
    case H_GE: return ">=";
    case H_GT: return ">";
    default:
      P_ERROR ("Unknown binary opcode %d", op);
      return "?";
    }
}

static const char *
h_unop_symbol (h_opcode op)
{
  switch (op)
    {
    case H_NOT: return "!";
    case H_NEG: return "-";
    case H_COMP: return "~";
    default:
      P_ERROR ("Unknown unary opcode %d", op);
      return "?";
    }
}

static void
gen_instr (const hir *ins)
{
  gen_indent ();

  switch (ins->op)
    {
    case H_LABEL:
      gen_operand (&ins->dst);
      gen (":");
      break;

    case H_JUMP:
      gen ("goto ");
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_CJUMP:
      gen ("ifFalse ");
      gen_operand (&ins->src1);
      gen (" goto ");
      gen_operand (&ins->src2);
      gen (";");
      break;

    case H_PARAM:
      gen ("param ");
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_CALL:
      gen_operand (&ins->dst);
      gen (" = call ");
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_RETURN:
      gen ("return");
      if (ins->src1.kind != HO_NONE)
        {
          space ();
          gen_operand (&ins->src1);
        }
      gen (";");
      break;

    case H_COPY:
      gen_operand (&ins->dst);
      gen (" = ");
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_GET:
      gen_operand (&ins->dst);
      gen (" = ");
      gen_operand (&ins->src1);
      begin_bracket ();
      gen_operand (&ins->src2);
      end_bracket ();
      gen (";");
      break;

    case H_SET:
      gen_operand (&ins->dst);
      begin_bracket ();
      gen_operand (&ins->src1);
      end_bracket ();
      gen (" = ");
      gen_operand (&ins->src2);
      gen (";");
      break;

    case H_ADDR:
      gen_operand (&ins->dst);
      gen (" = &");
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_NOT: case H_NEG: case H_COMP:
      gen_operand (&ins->dst);
      gen (" = %s", h_unop_symbol (ins->op));
      gen_operand (&ins->src1);
      gen (";");
      break;

    case H_INC: case H_DEC:
      gen_operand (&ins->dst);
      gen (" = ");
      gen_operand (&ins->src1);
      gen ("%s", ins->op == H_INC ? "++" : "--");
      gen (";");
      break;

    default:
      gen_operand (&ins->dst);
      gen (" = ");
      gen_operand (&ins->src1);
      gen (" %s ", h_binop_symbol (ins->op));
      gen_operand (&ins->src2);
      gen (";");
      break;
    }
}

void
gen_indent (void)
{
  int i;
  for (i = 0; i < indent; i++)
    fprintf (fout, "\t");
}

void
newline (void)
{
  fprintf (fout, "\n");
}

void
space (void)
{
  fprintf (fout, " ");
}

void
begin_bracket (void)
{
  gen ("[");
}

void
end_bracket (void)
{
  gen ("]");
}

void
gen (const char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  vfprintf (fout, msg, args);
  va_end (args);
}

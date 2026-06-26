#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "builtin_func.h"
#include "type/tytab.h"
#include "type/check.h"

/* forward declarations to avoid ast_sem.h/type/base.h struct conflict */
struct program;
extern void convert_program (ast_t *root, struct program **out);
extern void set_codegen_output (FILE *);
extern void gen_program (struct program *);

extern ast_t *compile (FILE *);

char *yyfilename;

int
main (int argc, char *argv[])
{
  ast_t *root;
  struct program *sem_analysis;
  FILE *f;

  if (argc > 1)
    {
      if ((f = fopen (argv[1], "r")) == NULL)
        {
          fprintf (stderr, "file open error for %s\n", argv[1]);
          exit (1);
        }
      yyfilename = argv[1];
    }
  else
    {
      f = stdin;
    }

  init_type_table ();
  register_builtin_functions ();

  root = compile (f);
  if (!root)
    exit (0);

  /* Layer 3: annotate AST with types before lowering */
  {
    check_ctx_t ctx;
    ctx.fn_ret = NULL;
    ctx.delta = NULL;
    synth_expr (&ctx, root);
  }

  convert_program (root, &sem_analysis);
  set_codegen_output (stdout);
  gen_program (sem_analysis);

  exit (0);
}

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "builtin_func.h"
#include "type/check.h"
#include "type/tytab.h"

#define HTAB "\t"
#define NEWLINE "\n"
#define FLAG_COMPILE (1)
#define FLAG_OUTPUT (1 << 1)

struct program;
extern void convert_program (ast_t *root, struct program **out);
extern void set_codegen_output (FILE *);
extern void gen_program (struct program *);

extern ast_t *compile (FILE *);

char *yyfilename;
int flags;

static struct option cmd_opts[] = { { "help", no_argument, 0, 'h' },
                                    { "compile", required_argument, 0, 'c' },
                                    { "output", required_argument, 0, 'o' } };

static void
print_usage (const char *file_name)
{
  printf ("Usage: %s [OPTION]..." NEWLINE, file_name);
  printf (HTAB "--help : Specify the source file to compile [-h]" NEWLINE);
  printf (HTAB "--compile : Specify the source file to compile [-c]" NEWLINE);
}

int
main (int argc, char *argv[])
{
  ast_t *root;
  struct program *sem_analysis;
  FILE *f, *o;
  int opt;
  optind = 1;
  flags = 0;

  while ((opt = getopt_long (argc, argv, "hc:")) != -1)
    {
      switch (opt)
        {
        case 'h':
          print_usage (argv[0]);
          return 0;
        case 'c':

          if (optind >= argc)
            {
              fprintf (stderr, "Compile option(-c) requires an argument.\n");
              return -1;
            }

          if ((f = fopen (argv[optind], "r")) == NULL)
            {
              fprintf (stderr, "%s does not exist.\n", argv[optind]);
              return -1;
            }

          flags |= FLAG_COMPILE;
          break;
        case 'o':

          if (optind >= argc)
            {
              fprintf (stderr, "Output option(-o) requires an argument.\n");
              return -1;
            }

          if ((o = fopen (argv[optind], "w")) == NULL)
            {
              fprintf (stderr, "%s does not exist.\n", argv[optind]);
              return -1;
            }

          flags |= FLAG_OUTPUT;
          break;
        default:
          break;
        }
    }

  if (!(flags & FLAG_COMPILE))
    {
      fprintf (stderr, "Specify a compile target");
      return -1;
    }

  init_type_table ();
  register_builtin_functions ();

  root = compile (f);
  if (!root)
    exit (0);

  {
    check_ctx_t ctx;
    ctx.fn_ret = NULL;
    ctx.delta = NULL;
    synth_expr (&ctx, root);
  }

  convert_program (root, &sem_analysis);

  if (flags & FLAG_OUTPUT)
    {
      set_codegen_output (o);
    }
  else
    {
      set_codegen_output (stdout);
    }

  gen_program (sem_analysis);

  return 0;
}

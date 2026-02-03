#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_

#define NAMELEN 2048

enum directive_kind
{
  DIR_IF,
  DIR_DEFINE,
  DIR_IFNDEF,
  DIR_IFDEF,
  DIR_ELIF,
  DIR_INCLUDE,
  DIR_OPENQASM
};

enum placeholder_kind
{
  PH_TEXT,
  PH_PLACEHOLDER,
  PH_STRINGIFIED
};

enum if_op
{
  IF_L,
  IF_G,
  IF_LE,
  IF_GE,
  IF_EQ,
  IF_NE,
  IF_AND,
  IF_OR
};

enum operand_kind
{
  OP_INTEGER,
  OP_FLOAT
};

struct if_state
{
};

struct placeholder
{
  enum placeholder_kind kind;
  char name[NAMELEN];
  struct placeholder *next;
  struct placeholder *prev;
};

struct macro_args
{
  char *name;

  struct macro_args *prev;
  struct macro_args *next;
};

struct dir_openqasm
{
  int version;
};

struct dir_define
{
  char name[NAMELEN];
  struct macro_args *args;
  struct placeholder *content;
};

struct directive
{
  enum directive_kind kind;

  union
  {
    struct dir_define *define;
    struct dir_openqasm *openqasm;
  } value;

  struct directive *prev;
};

struct operand
{
  enum operand_kind kind;
  union
  {
    int i;
    float f;
  } value;
};

struct dir_define *find_macro (const char *name);

int validate_expr (enum if_op op, struct operand *l, struct operand *r);

int expand_placeholder (char **out, struct placeholder *body);

void dir_if (int val);
void dir_ifdef (int val);
void dir_ifndef (int val);
void dir_endif ();

struct dir_define *new_define (const char *name);
struct dir_define *top_define ();
int is_define_arg (struct macro_args *arg_list, const char *name);
void push_define (struct dir_define *define);
void pop_define ();

struct macro_args *args_builder_end (struct macro_args *chain,
                                     const char *name);
struct macro_args *args_builder_append (struct macro_args *chain,
                                        const char *name);
struct macro_args *args_builder_begin (const char *name);

struct placeholder *ph_builder_append (struct placeholder *chain,
                                       enum placeholder_kind kind,
                                       const char *name);
struct placeholder *ph_builder_begin (enum placeholder_kind kind,
                                      const char *name);
struct placeholder *ph_builder_end (struct placeholder *chain,
                                    enum placeholder_kind kind,
                                    const char *name);
struct placeholder *ph_builder_concat (struct placeholder *chain,
                                       struct placeholder *item);

struct dir_openqasm *openqasm_new (int version);

void set_skip_mode (int skip);
int get_skip_mode (void);

#endif

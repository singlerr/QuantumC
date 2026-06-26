#include "builtin_func.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct builtin_func
{
  char *name;
  void (*convert) (app_t *, struct expression **);
  struct builtin_func *prev;
};

struct builtin_func *func_registry = NULL;

#define register_func(func_name)                                              \
  do                                                                          \
    {                                                                         \
      extern void convert_builtin_##func_name (app_t *,                      \
                                               struct expression **);         \
      struct builtin_func *func                                               \
          = (struct builtin_func *)malloc (sizeof (struct builtin_func));     \
      func->name = #func_name;                                               \
      func->convert = &convert_builtin_##func_name;                          \
      func->prev = func_registry;                                             \
      func_registry = func;                                                   \
    }                                                                         \
  while (0)

#define register_gate(gate) register_func (apply_##gate)

int
convert_builtin_function (const char *name, app_t *postfix,
                          struct expression **out)
{
  struct builtin_func *func;

  for (func = func_registry; func; func = func->prev)
    {
      if (strcmp (func->name, name) == 0)
        {
          func->convert (postfix, out);
          return 1;
        }
    }
  return 0;
}

void
register_builtin_functions ()
{
  register_gate (X);
  register_gate (Y);
  register_gate (Z);
  register_gate (S);
  register_gate (T);
  register_gate (H);
  register_gate (CX);
  register_gate (CZ);
  register_gate (CNOT);
  register_gate (CCX);
  register_gate (RX);
  register_gate (RY);
  register_gate (RZ);
  register_func (measure);
}

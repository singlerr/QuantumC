#ifndef _BUILTIN_FUNC_H_
#define _BUILTIN_FUNC_H_

/* forward-declare app_t to avoid type/base.h and ast_sem.h struct conflicts */
struct app;
typedef struct app app_t;

struct expression;

void register_builtin_functions ();
int convert_builtin_function (const char *name, app_t *func_call,
                              struct expression **out);

#define BUILTIN_FUNC(func_name)                                               \
  void convert_builtin_##func_name (app_t *func_call, struct expression **out)

#endif

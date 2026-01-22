#ifndef _BUILTIN_FUNC_H_
#define _BUILTIN_FUNC_H_

struct expression;
struct _sqz_expr_src_func_call;

void register_builtin_functions();
int convert_builtin_function(const char *name, struct _sqz_expr_src_func_call *, struct expression **);

#define BUILTIN_FUNC(func_name) void convert_builtin_##func_name(struct _sqz_expr_src_func_call *func_call, struct expression **out)
#endif
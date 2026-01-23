#include "builtin_func.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast_sqz.h"
#include "diagnostics.h"

struct builtin_func
{
    char *name;
    void (*convert)(struct _sqz_expr_src_func_call *, struct expression **);
    struct builtin_func *prev;
};

struct builtin_func *func_registry = NULL;

#define register_func(func_name)                                                                         \
    do                                                                                                   \
    {                                                                                                    \
        extern void convert_builtin_##func_name(struct _sqz_expr_src_func_call *, struct expression **); \
        struct builtin_func *func = (struct builtin_func *)malloc(sizeof(struct builtin_func));          \
        func->name = #func_name;                                                                         \
        func->convert = &convert_builtin_##func_name;                                                    \
        func->prev = func_registry;                                                                      \
        func_registry = func;                                                                            \
    } while (0)

#define register_gate(gate) register_func(apply_##gate)

int convert_builtin_function(const char *name, struct _sqz_expr_src_func_call *postfix, struct expression **out)
{
    struct builtin_func *func;

    for (func = func_registry; func; func = func->prev)
    {
        if (strcmp(func->name, name) == 0)
        {
            func->convert(postfix, out);
            return TRUE;
        }
    }

    return FALSE;
}

void register_builtin_functions()
{
    register_func(measure);
    register_gate(X);
    register_gate(Y);
    register_gate(Z);
    register_gate(S);
    register_gate(T);
    register_gate(H);
    register_gate(CNOT);
    register_gate(CZ);
    register_gate(CX);
    register_gate(CCX);
    register_gate(RX);
    register_gate(RY);
    register_gate(RZ);
}
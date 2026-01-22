#include <stdio.h>
#include "builtin_quantum.h"
#include "ast_sqz.h"
#include "ast_sem.h"
#include "builtin_func.h"
#include "diagnostics.h"

#define BUILTIN_GATE(GATE)                                          \
    BUILTIN_FUNC(apply_##GATE)                                      \
    {                                                               \
        expression_list *arg_list;                                  \
        expression_list *arg;                                       \
        expression *gate_expr = IALLOC(expression);                 \
        identifier *name;                                           \
        qubit_list *qubits = NULL;                                  \
        convert_expression_arguments(func_call->args, &arg_list);   \
        name = new_identifier(#GATE);                               \
        list_for_each_entry(arg, arg_list)                          \
        {                                                           \
            qubit_list *q = wrap_qubit_list(new_qubit(arg->value)); \
            if (!qubits)                                            \
            {                                                       \
                qubits = q;                                         \
            }                                                       \
            else                                                    \
            {                                                       \
                list_add(qubit_list, q, qubits);                    \
            }                                                       \
        }                                                           \
        list_goto_first(qubit_list, qubits);                        \
        gate_expr->as.quantum.quantum_gate.qubits = qubits;         \
        gate_expr->as.quantum.quantum_gate.name = name;             \
        gate_expr->kind = EXPR_QUANTUM_GATE;                        \
        *out = gate_expr;                                           \
    }

BUILTIN_GATE(X)
BUILTIN_GATE(Y)
BUILTIN_GATE(Z)
BUILTIN_GATE(S)
BUILTIN_GATE(V)
BUILTIN_GATE(H)
BUILTIN_GATE(CNOT)
BUILTIN_GATE(CZ)
BUILTIN_GATE(CX)
BUILTIN_GATE(DCNOT)

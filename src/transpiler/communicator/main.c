#include <stdio.h>
#include <stdlib.h>

#include "dispatcher.h"

int main(void)
{
    const char* qasm = 
        "OPENQASM 3.0; "
        "include \\\"stdgates.inc\\\"; "
        "bit[2] c; "
        "qreg q[2]; "
        "h q[0]; "
        "cx q[0], q[1]; "
        "c = measure q;";

    fprintf(stdout, "=== QuantumC Runtime ===\n");

    char* result = run_quantum_circuit(qasm);

    fprintf(stdout, "=== Final Result ===\n");

    free(result);

    return 0;
}

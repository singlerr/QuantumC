#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

// FIXME: Wrong addresses?
#define IBM_AUTH_URL "https://api.quantum-computing.ibm.com"
#define IBM_JOBS_URL "https://api.quantum-computing.ibm.com/runtime/jobs"
#define BACKEND_NAME "ibmq_qasm_simulator"

typedef struct responsebuf
{
    char* memory;
    size_t size;
} RESPONSEBUF;

char* run_quantum_circuit(const char* qasm_code);

#endif

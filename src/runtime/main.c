#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

#include "comm.h"
#include "auth.h"
#include "sender.h"
#include "receiver.h"
#include "reader.h"


// TODO: Implement cleanup using goto statement pattern. <---
// TODO: Tidy up the define macros.
// TODO: Change cJSON variable names.
// TODO: Rewrite error messages.
// TODO: Document the source code.
// TODO: Write Makefile script.
int main(int argc, char** argv) {
    int termination_status = EXIT_FAILURE;

    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    // Check the input.

    if (argc != 2) {
        fprintf(stderr, "ERROR - One argument for the OpenQASM filename needed.\n");
        goto terminate;
    }

    char* qasm_filename = argv[1];

    // Read `config.json`.

    CONFIG* config = read_config(CONFIG_FILENAME);
    if (!config) {
        fprintf(stderr, "ERROR - Reading the config file failed!\n");
        goto terminate;
    }

    char* api_key = config->api_key;
    char* crn = config->crn;

    // Read the specified OpenQASM file.

    char* qasm = read_qasm(qasm_filename);
    if (!qasm) {
        fprintf(stderr, "ERROR - Reading the OpenQASM code failed!\n");
        goto cleanup_config;
    }

    fprintf(stdout, "OpenQASM Code: \n%s\n", qasm);

    // Configure and start authentication thread.

    TOKEN_DATA* token_data = (TOKEN_DATA*)calloc(1, sizeof(TOKEN_DATA));
    if (!token_data) {
        fprintf(stderr, "ERROR - Allocating memory for token data failed!\n");
        goto cleanup_qasm;
    }
    initialize_token_data(token_data, api_key);

    pthread_t authenticator_thread;
    void* authenticator_retval;    

    int create_status = pthread_create(&authenticator_thread, NULL, authenticator, (void*)token_data);
    if (create_status) {
        fprintf(stderr, "ERROR - Thread creation failed!\n");
        goto cleanup_token_data;
    }

    // Send a job to a quantum backend.

    char* job_id = sender(token_data, crn, qasm);
    if (!job_id) {
        fprintf(stderr, "ERROR - Job submission failed!\n");
        goto cleanup_token_data;
    }

    printf("Job ID: %s\n", job_id);

    // Receive the job result from the quantum backend.

    char* job_result = receiver(token_data, crn, job_id);
    if (!job_result) {
        fprintf(stderr, "ERROR - Job retrieval failed!\n");
        goto cleanup_job_id;
    }

    signal_job_terminated(token_data);

    // Join the authenticator thread to the main thread.

    int join_status = pthread_join(authenticator_thread, &authenticator_retval);
    if (join_status) {
        fprintf(stderr, "ERROR - Thread joined with an error!\n");
        goto cleanup_job_result;
    }

    fprintf(stdout, "=== Final Result ===\n\n");
    fprintf(stdout, "%s\n", job_result);

    termination_status = EXIT_SUCCESS;

    // Clean up.

cleanup_job_result:
    free(job_result);

cleanup_job_id:
    free(job_id);

cleanup_token_data:
    destroy_token_data(token_data);

cleanup_qasm:
    free(qasm);

cleanup_config:
    free(api_key);
    free(crn);
    free(config);

terminate:
    return termination_status;
}

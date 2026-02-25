#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

#include "comm.h"
#include "auth.h"
#include "sender.h"
#include "receiver.h"
#include "reader.h"


/**
 * @brief Entry point for the QuantumC runtime
 *
 * Reads configuration and OpenQASM input, starts the authenticator thread,
 * submits a job to the quantum backend, and retrieves and displays the result.
 *
 * @param argc Argument count (expects exactly 2: program name and OpenQASM filename)
 * @param argv Argument vector (argv[1] must be the OpenQASM filename)
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char** argv) {
    int termination_status = EXIT_FAILURE;

    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    // Check the input.

    if (argc != 2) {
        fprintf(stderr, "ERROR - One argument for the OpenQASM filename needed in main()!\n");
        goto terminate;
    }

    char* qasm_filename = argv[1];

    // Read config.json.

    CONFIG* config = read_config(CONFIG_FILENAME);
    if (!config) {
        fprintf(stderr, "ERROR - Reading the config file failed in main()!\n");
        goto terminate;
    }

    char* key = config->key;
    char* crn = config->crn;

    // Read the specified OpenQASM file.

    char* qasm = read_qasm(qasm_filename);
    if (!qasm) {
        fprintf(stderr, "ERROR - Reading the OpenQASM code failed in main()!\n");
        goto cleanup_config;
    }

    fprintf(stdout, "OpenQASM Code: \n%s\n", qasm);

    // Configure and start authentication thread.

    TOKEN_DATA* token_data = (TOKEN_DATA*)calloc(1, sizeof(TOKEN_DATA));
    if (!token_data) {
        fprintf(stderr, "ERROR - Allocating memory for token data failed in main()!\n");
        goto cleanup_qasm;
    }
    initialize_token_data(token_data, key);

    pthread_t authenticator_thread;
    void* authenticator_retval;    

    int create_status = pthread_create(&authenticator_thread, NULL, authenticator, (void*)token_data);
    if (create_status) {
        fprintf(stderr, "ERROR - Thread creation failed in main()!\n");
        goto cleanup_token_data;
    }

    // Send a job to a quantum backend.

    char* job_id = sender(token_data, crn, qasm);
    if (!job_id) {
        fprintf(stderr, "ERROR - Job submission failed in main()!\n");
        goto cleanup_token_data;
    }

    fprintf(stdout, "Job ID: %s\n\n", job_id);

    // Receive the job result from the quantum backend.

    char* job_result = receiver(token_data, crn, job_id);
    if (!job_result) {
        fprintf(stderr, "ERROR - Job retrieval failed in main()!\n");
        goto cleanup_job_id;
    }

    signal_job_terminated(token_data);

    // Join the authenticator thread to the main thread.

    int join_status = pthread_join(authenticator_thread, &authenticator_retval);
    if (join_status) {
        fprintf(stderr, "ERROR - Thread joined with an error in main()!\n");
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
    free(key);
    free(crn);
    free(config);

terminate:
    return termination_status;
}

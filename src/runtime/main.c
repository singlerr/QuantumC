#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

#include "comm.h"
#include "auth.h"
#include "sender.h"
#include "receiver.h"
#include "reader.h"


// TODO: Implement cleanup by using goto statement on entire source.
// TODO: Tidy up the define macros.
// TODO: Change cJSON variable names.
// TODO: Reorder the include statements. <--
// TODO: Rewrite error messages.
// TODO: Document the source code.
// TODO: Write Makefile script.
int main(int argc, char** argv) {
    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    // Check the input.

    if (argc != 2) {
        fprintf(stderr, "ERROR - Only one argument for the OpenQASM filename needed.\n");
        return EXIT_FAILURE;
    }

    char* qasm_filename = argv[1];

    // Read `config.json`.

    CONFIG* config = read_config(CONFIG_FILENAME);
    if (!config) {
        fprintf(stderr, "ERROR - Reading the config file failed!\n");
        return EXIT_FAILURE;
    }

    char* api_key = config->api_key;
    char* crn = config->crn;

    // Read the specified OpenQASM file.

    char* qasm = read_qasm(qasm_filename);
    if (!qasm) {
        fprintf(stderr, "ERROR - Reading the OpenQASM code failed!\n");
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }

    fprintf(stdout, "OpenQASM Code: \n%s\n", qasm);

    // Configure and start authentication thread.

    TOKEN_DATA* token_data = (TOKEN_DATA*)calloc(1, sizeof(TOKEN_DATA));
    if (!token_data) {
        fprintf(stderr, "ERROR - Allocating memory for token data failed!\n");
        free(qasm);
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }
    initialize_token_data(token_data, api_key);

    pthread_t authenticator_thread;
    void* authenticator_retval;    

    int create_status = pthread_create(&authenticator_thread, NULL, authenticator, (void*)token_data);
    if (create_status) {
        fprintf(stderr, "ERROR - Thread creation failed!\n");
        destroy_token_data(token_data);
        free(qasm);
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }

    // Send a job to a quantum backend.

    char* job_id = sender(token_data, crn, qasm);
    if (!job_id) {
        fprintf(stderr, "ERROR - Job submission failed!\n");
        destroy_token_data(token_data);
        free(qasm);
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }

    printf("Job ID: %s\n", job_id);

    // Receive the job result from the quantum backend.

    char* result = receiver(token_data, crn, job_id);
    if (!result) {
        fprintf(stderr, "ERROR - Job retrieval failed!\n");
        free(job_id);
        destroy_token_data(token_data);
        free(qasm);
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }

    signal_job_terminated(token_data);

    // Join the authenticator thread to the main thread.

    int join_status = pthread_join(authenticator_thread, &authenticator_retval);
    if (join_status) {
        fprintf(stderr, "ERROR - Thread joined with an error!\n");
        free(result);
        free(job_id);
        destroy_token_data(token_data);
        free(qasm);
        free(api_key);
        free(crn);
        free(config);
        return EXIT_FAILURE;
    }

    fprintf(stdout, "=== Final Result ===\n\n");
    fprintf(stdout, "%s\n", result);

    // Clean up.

    free(result);
    free(job_id);
    destroy_token_data(token_data);
    free(qasm);
    free(api_key);
    free(crn);
    free(config);

    return EXIT_SUCCESS;
}

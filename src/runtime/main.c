#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "comm.h"
#include "auth.h"
#include "sender.h"


int main(void)
{
    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    // Parsing `config.json`.

    FILE* file = fopen("config.json", "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening config.json failed!\n");
        return EXIT_FAILURE;
    }

    // TODO: Implement overflow checking.
    // TODO: Even better, implement dynamic size allocation.
    char* buffer = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!buffer) {
        fprintf(stderr, "ERROR - Allocating memory for buffer failed!\n");
        return EXIT_FAILURE;
    }

    fread(buffer, sizeof(char), BUFFER_NMEMB, file);
    fclose(file);

    cJSON* cjson_config = cJSON_Parse(buffer);
    if (!cjson_config) {
        fprintf(stderr, "ERROR - Parsing configuration JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) {
            fprintf(stderr, "ERROR - %s\n", error);
        }
        free(buffer);
        cJSON_Delete(cjson_config);
        return EXIT_FAILURE;
    }

    char* api = NULL;
    cJSON* cjson_api = cJSON_GetObjectItemCaseSensitive(cjson_config, "api");
    if (cJSON_IsString(cjson_api) && cjson_api->valuestring) {
        api = cjson_api->valuestring;
    } else {
        fprintf(stderr, "ERROR - Parsing API key failed!\n");
        free(buffer);
        cJSON_Delete(cjson_config);
        return EXIT_FAILURE;
    }

    char* crn = NULL;
    cJSON* cjson_crn = cJSON_GetObjectItemCaseSensitive(cjson_config, "crn");
    if (cJSON_IsString(cjson_crn) && cjson_crn->valuestring) {
        crn = cjson_crn->valuestring;
    } else {
        fprintf(stderr, "ERROR - Parsing CRN failed!\n");
        free(buffer);
        cJSON_Delete(cjson_config);
        return EXIT_FAILURE;
    }

    // Start the authenticator thread.

    pthread_t authenticator;

    TOKEN_DATA* token_data = (TOKEN_DATA*)calloc(1, sizeof(TOKEN_DATA));
    if (!token_context) {
        fprintf(stderr, "ERROR - Allocating memory for token data failed!\n");
        return;
    }

    token_data->api_key = (const char*)api;
    token_data->token = NULL;
    pthread_mutex_init(&token_data->lock, NULL);

    int create_status = pthread_create(&authenticator, NULL, start_authenticator, (void*)token_data);
    if (create_status) {
        fprintf(stderr, "ERROR - Thread creation failed!\n");
        pthread_mutex_destroy(&token_data->lock);
        free(token_data);
        return EXIT_FAILURE;
    }

    // Send a job to a quantum backend.

    int join_status = pthread_join(authenticator, NULL);
    if (join_status) {
        fprintf(stderr, "ERROR - Thread joining failed!\n");
        pthread_mutex_destroy(&token_data->lock);
        free(token_data);
        return EXIT_FAILURE;
    }

    // Parse the job result and display.

    // fprintf(stdout, "=== Final Result ===\n\n");

    // Clean up.

    free(buffer);
    pthread_mutex_destroy(&token_data->lock);
    free(token_data);
    cJSON_Delete(cjson_config);

    return EXIT_SUCCESS;
}

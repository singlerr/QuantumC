#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "comm.h"
#include "sender.h"


int main(void)
{
    fprintf(stdout, "=== QuantumC Runtime ===\n\n");

    FILE* file = fopen("config.json", "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening config.json failed!\n");
        return EXIT_FAILURE;
    }

    // TODO: Implement overflow checking.
    // TODO: Even better, implement dynamic size allocation.
    char* buffer = (char*)calloc(BUFFER_NMEMB, sizeof(char));
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

    char* response = authenticate(api, crn);
    if (response) {
        fprintf(stdout, "A backend was successfully chosen!\n");
        fprintf(stdout, "Chosen Backend: %s\n", response);
    }

    free(buffer);
    cJSON_Delete(cjson_config);
    free(response);

    // fprintf(stdout, "=== Final Result ===\n\n");

    return EXIT_SUCCESS;
}

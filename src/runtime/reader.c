#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "reader.h"


int count_characters(char* filename) {
    int count = -1;

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed!\n", filename);
        goto terminate;
    }

    count = 0;
    for (char ch = getc(file); ch != EOF; ch = getc(file)) {
        count++;
    }

    fclose(file);

terminate:
    return count;
}


CONFIG* read_config(char* filename) {
    CONFIG* config = NULL;

    int characters = count_characters(filename);
    if (characters <= 0) {
        fprintf(stderr, "ERROR - Counting the number of characters failed!\n");
        goto terminate;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed!\n", filename);
        goto terminate;
    }

    char* buffer = (char*)calloc(characters+1, sizeof(char));
    if (!buffer) {
        fprintf(stderr, "ERROR - Allocating memory for buffer failed!\n");
        goto cleanup_file;
    }

    fread(buffer, sizeof(char), characters, file);

    cJSON* cjson_config = cJSON_Parse(buffer);
    if (!cjson_config) {
        fprintf(stderr, "ERROR - Parsing configuration JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) {
            fprintf(stderr, "ERROR - %s\n", error);
        }
        goto cleanup_buffer;
    }

    cJSON* cjson_api = cJSON_GetObjectItemCaseSensitive(cjson_config, "api");
    if (!cJSON_IsString(cjson_api) || !cjson_api->valuestring) {
        fprintf(stderr, "ERROR - Parsing API key failed!\n");
        goto cleanup_cjson_config;
    }

    cJSON* cjson_crn = cJSON_GetObjectItemCaseSensitive(cjson_config, "crn");
    if (!cJSON_IsString(cjson_crn) || !cjson_crn->valuestring) {
        fprintf(stderr, "ERROR - Parsing CRN failed!\n");
        goto cleanup_cjson_config;
    }

    config = (CONFIG*)calloc(1, sizeof(CONFIG));
    if (!config) {
        fprintf(stderr, "ERROR - Memory allocation for config failed!\n");
        goto cleanup_cjson_config;
    }

    config->api_key = strdup(cjson_api->valuestring);
    config->crn = strdup(cjson_crn->valuestring);

cleanup_cjson_config:
    if (cjson_config) cJSON_Delete(cjson_config);

cleanup_buffer:
    if (buffer) free(buffer);

cleanup_file:
    if (file) fclose(file);

terminate:
    return config;
}

char* read_qasm(char* filename) {
    char* qasm = NULL;

    int characters = count_characters(filename);
    if (characters <= 0) {
        fprintf(stderr, "ERROR - Counting the number of characters failed!\n");
        goto terminate;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed!\n", filename);
        goto terminate;
    }

    qasm = (char*)calloc(characters+1, sizeof(char));
    if (!qasm) {
        fprintf(stderr, "ERROR - Allocating memory for QASM code failed!\n");
        goto cleanup_file;
    }

    fread(qasm, sizeof(char), characters, file);
    
cleanup_file:
    fclose(file);

terminate:
    return qasm;
}

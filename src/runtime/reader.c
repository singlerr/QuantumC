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

    cJSON* config_cjson = cJSON_Parse(buffer);
    if (!config_cjson) {
        fprintf(stderr, "ERROR - Parsing configuration JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto cleanup_buffer;
    }

    cJSON* cjson_api = cJSON_GetObjectItemCaseSensitive(config_cjson, "api");
    if (!cJSON_IsString(cjson_api) || !cjson_api->valuestring) {
        fprintf(stderr, "ERROR - Parsing API key failed!\n");
        goto cleanup_config_cjson;
    }

    cJSON* cjson_crn = cJSON_GetObjectItemCaseSensitive(config_cjson, "crn");
    if (!cJSON_IsString(cjson_crn) || !cjson_crn->valuestring) {
        fprintf(stderr, "ERROR - Parsing CRN failed!\n");
        goto cleanup_config_cjson;
    }

    config = (CONFIG*)calloc(1, sizeof(CONFIG));
    if (!config) {
        fprintf(stderr, "ERROR - Memory allocation for config failed!\n");
        goto cleanup_config_cjson;
    }

    config->api_key = strdup(cjson_api->valuestring);
    config->crn = strdup(cjson_crn->valuestring);

cleanup_config_cjson:
    cJSON_Delete(config_cjson);

cleanup_buffer:
    free(buffer);

cleanup_file:
    fclose(file);

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

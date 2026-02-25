#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "reader.h"


/**
 * @brief Count the number of characters in a file
 *
 * Opens the file and counts all characters until EOF.
 *
 * @param filename Path to the file to count
 * @return Number of characters on success, or -1 on failure
 */
int count_characters(char* filename) {
    int count = -1;

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed in count_characters()!\n", filename);
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


/**
 * @brief Read and parse JSON configuration file
 *
 * Reads a JSON configuration file and extracts the API key and CRN values
 * into a newly allocated CONFIG structure.
 *
 * @param filename Path to the configuration file (JSON format)
 * @return Pointer to newly allocated CONFIG (CALLER MUST FREE) or NULL on failure
 */
CONFIG* read_config(char* filename) {
    CONFIG* config = NULL;

    int characters = count_characters(filename);
    if (characters <= 0) {
        fprintf(stderr, "ERROR - Counting the number of characters failed in read_config()!\n");
        goto terminate;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed in read_config()!\n", filename);
        goto terminate;
    }

    char* buffer = (char*)calloc(characters+1, sizeof(char));
    if (!buffer) {
        fprintf(stderr, "ERROR - Allocating memory for buffer failed in read_config()!\n");
        goto cleanup_file;
    }

    fread(buffer, sizeof(char), characters, file);

    cJSON* config_cjson = cJSON_Parse(buffer);
    if (!config_cjson) {
        fprintf(stderr, "ERROR - Parsing configuration JSON failed in read_config()!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto cleanup_buffer;
    }

    cJSON* key_cjson = cJSON_GetObjectItemCaseSensitive(config_cjson, "key");
    if (!cJSON_IsString(key_cjson) || !key_cjson->valuestring) {
        fprintf(stderr, "ERROR - Parsing API key failed in read_config()!\n");
        goto cleanup_config_cjson;
    }

    cJSON* crn_cjson = cJSON_GetObjectItemCaseSensitive(config_cjson, "crn");
    if (!cJSON_IsString(crn_cjson) || !crn_cjson->valuestring) {
        fprintf(stderr, "ERROR - Parsing CRN failed in read_config()!\n");
        goto cleanup_config_cjson;
    }

    config = (CONFIG*)calloc(1, sizeof(CONFIG));
    if (!config) {
        fprintf(stderr, "ERROR - Memory allocation for config failed in read_config()!\n");
        goto cleanup_config_cjson;
    }

    config->key = strdup(key_cjson->valuestring);
    config->crn = strdup(crn_cjson->valuestring);

cleanup_config_cjson:
    cJSON_Delete(config_cjson);

cleanup_buffer:
    free(buffer);

cleanup_file:
    fclose(file);

terminate:
    return config;
}

/**
 * @brief Read OpenQASM file contents into a string
 *
 * Reads the entire contents of an OpenQASM file into a newly allocated
 * null-terminated string.
 *
 * @param filename Path to the OpenQASM file
 * @return Newly allocated string (CALLER MUST FREE) or NULL on failure
 */
char* read_qasm(char* filename) {
    char* qasm = NULL;

    int characters = count_characters(filename);
    if (characters <= 0) {
        fprintf(stderr, "ERROR - Counting the number of characters failed in read_qasm()!\n");
        goto terminate;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR - Opening %s failed in read_qasm()!\n", filename);
        goto terminate;
    }

    qasm = (char*)calloc(characters+1, sizeof(char));
    if (!qasm) {
        fprintf(stderr, "ERROR - Allocating memory for QASM code failed in read_qasm()!\n");
        goto cleanup_file;
    }

    fread(qasm, sizeof(char), characters, file);
    
cleanup_file:
    fclose(file);

terminate:
    return qasm;
}

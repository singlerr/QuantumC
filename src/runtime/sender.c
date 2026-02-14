#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "comm.h"
#include "sender.h"


char* get_backends_data(TOKEN_DATA* token_data, char* crn) {
    char* token = copy_bearer_token(token_data);

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        free(token);
        return NULL;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    struct curl_slist* headers = NULL;

    // TODO: Implement overflow checking.
    // TODO: Even better, implement dynamic size allocation.
    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));

    snprintf(token_header, BUFFER_NMEMB, "Authorization: Bearer %s", token);
    snprintf(crn_header, BUFFER_NMEMB, "Service-CRN: %s", crn);

    headers = curl_slist_append(headers, "accept: application/json");
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, crn_header);
    headers = curl_slist_append(headers, IBM_API_VERSION);

    curl_easy_setopt(curl, CURLOPT_URL, "https://quantum.cloud.ibm.com/api/v1/backends");
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    CURLcode response = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Getting backend information failed!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        free(token);
        free(rb.data);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(token_header);
        free(crn_header);
        return NULL;
    }

    free(token);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(token_header);
    free(crn_header);

    return rb.data;
}

char* select_backend(char* backends_data) {
    cJSON* cjson_backends_data = cJSON_Parse(backends_data);
    if (!cjson_backends_data) {
        fprintf(stderr, "ERROR - Parsing backends data JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) {
            fprintf(stderr, "ERROR - %s\n", error);
        }
        cJSON_Delete(cjson_backends_data);
        return NULL;
    }

    cJSON* cjson_devices = cJSON_GetObjectItemCaseSensitive(cjson_backends_data, "devices");
    if (!cjson_devices || !cjson_devices->child) {
        fprintf(stderr, "ERROR - Parsing devices list failed!\n");
        cJSON_Delete(cjson_backends_data);
        return NULL;
    }

    int least_busy_device_queue = INT_MAX;
    char* least_busy_device_name = NULL;

    for (cJSON* cjson_device = cjson_devices->child; cjson_device; cjson_device = cjson_device->next) {
        int device_jobs = 0;

        cJSON* cjson_device_jobs = cJSON_GetObjectItemCaseSensitive(cjson_device, "queue_length");
        if (cJSON_IsNumber(cjson_device_jobs)) {
            device_jobs = cjson_device_jobs->valueint;
        } else {
            fprintf(stderr, "ERROR - Parsing the queue length failed!\n");
            cJSON_Delete(cjson_backends_data);
            return NULL;
        }
        
        if (0 <= device_jobs && device_jobs < least_busy_device_queue) {
            least_busy_device_queue = device_jobs;

            cJSON* cjson_device_name = cJSON_GetObjectItemCaseSensitive(cjson_device, "name");
            if (cJSON_IsString(cjson_device_name) && cjson_device_name->valuestring) {
                least_busy_device_name = cjson_device_name->valuestring;
            } else {
                fprintf(stderr, "ERROR - Parsing the device name failed!\n");
                cJSON_Delete(cjson_backends_data);
                return NULL;
            }
        }
    }

    char* chosen_backend = (char*)calloc(strlen(least_busy_device_name)+1, sizeof(char));
    strcpy(chosen_backend, least_busy_device_name);

    cJSON_Delete(cjson_backends_data);

    return chosen_backend;
}

char* submit_job(TOKEN_DATA* token_data, char* crn, char* backend, char* qasm) {
    char* token = copy_bearer_token(token_data);

    free(token);

    return "";
}


// Returns job ID.
char* sender(TOKEN_DATA* token_data, char* crn, char* qasm) {
    while (!token_data->token) {
        pthread_cond_wait(&token_data->token_received, &token_data->lock);
    }

    char* backends_data = get_backends_data(token_data, crn);
    if (!backends_data) {
        fprintf(stderr, "ERROR - Fetching backends data failed!\n");
        return NULL;
    }

    char* backend = select_backend(backends_data);
    if (!backend) {
        fprintf(stderr, "ERROR - Selecting backend device failed!\n");
        free(backends_data);
        return NULL;
    }

    char* job_id = submit_job(token_data, crn, backend, qasm);

    free(backends_data);
    free(backend);

    return job_id;
}

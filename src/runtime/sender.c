#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "comm.h"
#include "sender.h"


char* get_backends_data(TOKEN_DATA* token_data, char* crn) {
    char* backends_data = NULL;

    char* token = copy_bearer_token(token_data);
    if (!token) {
        fprintf(stderr, "ERROR - Copying the bearer token failed!\n");
        goto terminate;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        goto cleanup_token;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed1\n");
        goto cleanup_curl;
    }

    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!token_header) {
        fprintf(stderr, "ERROR - Allocating memory for token header failed!\n");
        goto cleanup_rb;
    }

    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!crn_header) {
        fprintf(stderr, "ERROR - Allocating memory for CRN header failed!\n");
        goto cleanup_token_header;
    }

    snprintf(token_header, BUFFER_NMEMB, "Authorization: Bearer %s", token);
    snprintf(crn_header, BUFFER_NMEMB, "Service-CRN: %s", crn);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, crn_header);
    headers = curl_slist_append(headers, "IBM-API-Version: 2026-02-01");
    if (!headers) {
        fprintf(stderr, "ERROR - Content type appending failed!\n");
        goto cleanup_crn_header;
    }

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
        goto cleanup_headers;
    }

    backends_data = strdup(rb.data);

cleanup_headers:
    curl_slist_free_all(headers);

cleanup_crn_header:
    free(crn_header);

cleanup_token_header:
    free(token_header);

cleanup_rb:
    free(rb.data);

cleanup_curl:
    curl_easy_cleanup(curl);

cleanup_token:
    free(token);

terminate:
    return backends_data;
}

char* select_backend(char* backends_data) {
    char* backend = NULL;

    cJSON* backends_data_cjson = cJSON_Parse(backends_data);
    if (!backends_data_cjson) {
        fprintf(stderr, "ERROR - Parsing backends data JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* cjson_devices = cJSON_GetObjectItemCaseSensitive(backends_data_cjson, "devices");
    if (!cjson_devices || !cjson_devices->child) {
        fprintf(stderr, "ERROR - Parsing devices list failed!\n");
        goto cleanup_backends_data_cjson;
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
            goto cleanup_backends_data_cjson;
        }
        
        if (0 <= device_jobs && device_jobs < least_busy_device_queue) {
            least_busy_device_queue = device_jobs;

            cJSON* cjson_device_name = cJSON_GetObjectItemCaseSensitive(cjson_device, "name");
            if (cJSON_IsString(cjson_device_name) && cjson_device_name->valuestring) {
                least_busy_device_name = cjson_device_name->valuestring;
            } else {
                fprintf(stderr, "ERROR - Parsing the device name failed!\n");
                goto cleanup_backends_data_cjson;
            }
        }
    }

    backend = strdup(least_busy_device_name);

cleanup_backends_data_cjson:
    cJSON_Delete(backends_data_cjson);

terminate:
    return backend;
}

char* build_payload(char* backend, char* qasm) {
    char* payload = NULL;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "program_id", "sampler");
    cJSON_AddStringToObject(root, "backend", backend);

    cJSON* params = cJSON_AddObjectToObject(root, "params");
    cJSON* pubs = cJSON_AddArrayToObject(params, "pubs");
    cJSON* single_pub = cJSON_CreateArray();
    cJSON_AddItemToArray(single_pub, cJSON_CreateString(qasm));
    cJSON_AddItemToArray(pubs, single_pub);

    cJSON* options = cJSON_AddObjectToObject(params, "options");
    cJSON* dd = cJSON_AddObjectToObject(options, "dynamical_decoupling");
    cJSON_AddBoolToObject(dd, "enable", cJSON_True);

    cJSON_AddNumberToObject(params, "version", 2);

    payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return payload;
}

char* submit_job(TOKEN_DATA* token_data, char* crn, char* payload) {
    char* response_data = NULL;

    char* token = copy_bearer_token(token_data);
    if (!token) {
        fprintf(stderr, "ERROR - Copying the bearer token failed!\n");
        goto terminate;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        goto cleanup_token;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed!\n");
        goto cleanup_curl;
    }

    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!token_header) {
        fprintf(stderr, "ERROR - Allocating memory for token header failed!\n");
        goto cleanup_rb;
    }

    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!crn_header) {
        fprintf(stderr, "ERROR - Allocating memory for CRN header failed!\n");
        goto cleanup_token_header;
    }

    snprintf(token_header, BUFFER_NMEMB, "Authorization: Bearer %s", token);
    snprintf(crn_header, BUFFER_NMEMB, "Service-CRN: %s", crn);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, crn_header);
    headers = curl_slist_append(headers, "IBM-API-Version: 2026-02-01");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!headers) {
        fprintf(stderr, "ERROR - Content type appending failed!\n");
        goto cleanup_crn_header;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://quantum.cloud.ibm.com/api/v1/jobs");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    CURLcode response = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Job submission failed!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        if (rb.data && rb.size > 0) fprintf(stderr, "ERROR - Response body: %s\n", rb.data);
        goto cleanup_headers;
    }

    response_data = strdup(rb.data);

cleanup_headers:
    curl_slist_free_all(headers);

cleanup_crn_header:
    free(crn_header);

cleanup_token_header:
    free(token_header);

cleanup_rb:
    free(rb.data);

cleanup_curl:
    curl_easy_cleanup(curl);

cleanup_token:
    free(token);

terminate:
    return response_data;
}

char* parse_job_id(char* response) {
    char* job_id = NULL;

    cJSON* response_cjson = cJSON_Parse(response);
    if (!response_cjson) {
        fprintf(stderr, "ERROR - Parsing job submission response JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* job_id_cjson = cJSON_GetObjectItemCaseSensitive(response_cjson, "id");
    if (cJSON_IsString(job_id_cjson) && job_id_cjson->valuestring) {
        job_id = strdup(job_id_cjson->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing job ID from response failed!\n");
        goto cleanup_response_cjson;
    }

cleanup_response_cjson:
    cJSON_Delete(response_cjson);

terminate:
    return job_id;
}


char* sender(TOKEN_DATA* token_data, char* crn, char* qasm) {
    pthread_mutex_lock(&token_data->lock);
    while (!token_data->token_received_bool) {
        pthread_cond_wait(&token_data->token_received_cond, &token_data->lock);
    }
    pthread_mutex_unlock(&token_data->lock);

    char* job_id = NULL;

    char* backends_data = get_backends_data(token_data, crn);
    if (!backends_data) {
        fprintf(stderr, "ERROR - Fetching backends data failed!\n");
        goto terminate;
    }

    char* backend = select_backend(backends_data);
    if (!backend) {
        fprintf(stderr, "ERROR - Selecting the backend device failed!\n");
        goto cleanup_backends_data;
    }

    char* payload = build_payload(backend, qasm);
    if (!payload) {
        fprintf(stderr, "ERROR - Building a payload for submitting job failed!\n");
        goto cleanup_backend;
    }

    char* response = submit_job(token_data, crn, payload);
    if (!response) {
        fprintf(stderr, "ERROR - Getting a response from job submission failed!\n");
        goto cleanup_payload;
    }

    job_id = parse_job_id(response);
    if (!job_id) {
        fprintf(stderr, "ERROR - Parsing the job ID failed!\n");
        goto cleanup_response;
    }

cleanup_response:
    free(response);

cleanup_payload:
    free(payload);

cleanup_backend:
    free(backend);

cleanup_backends_data:
    free(backends_data);

terminate:
    return job_id;
}

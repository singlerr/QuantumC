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


/**
 * @brief Fetch available backends data from IBM Quantum API
 *
 * Uses the bearer token in token_data and the provided CRN to request the
 * list of backends. The returned string must be freed by the caller.
 *
 * @param token_data Pointer to TOKEN_DATA with authentication token
 * @param crn Service CRN string to include in headers
 * @return JSON response string on success (CALLER MUST FREE), or NULL on error
 */
char* get_backends_data(TOKEN_DATA* token_data, char* crn) {
    char* backends_data = NULL;

    char* token = copy_bearer_token(token_data);
    if (!token) {
        fprintf(stderr, "ERROR - Copying bearer token failed in get_backends_data()!\n");
        goto terminate;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR - cURL initialization failed in get_backends_data()!\n");
        goto cleanup_token;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed in get_backends_data()!\n");
        goto cleanup_curl;
    }

    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!token_header) {
        fprintf(stderr, "ERROR - Allocating memory for token header failed in get_backends_data()!\n");
        goto cleanup_rb;
    }

    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!crn_header) {
        fprintf(stderr, "ERROR - Allocating memory for CRN header failed in get_backends_data()!\n");
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
        fprintf(stderr, "ERROR - Header construction failed in get_backends_data()!\n");
        goto cleanup_crn_header;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://quantum.cloud.ibm.com/api/v1/backends");
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    CURLcode response_code = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response_code != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Getting backend information failed in get_backends_data()!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response_code));
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

/**
 * @brief Select the least-busy backend from backends JSON
 *
 * Parses the provided JSON and returns a duplicated backend name chosen by
 * queue length heuristics.
 *
 * @param backends_data JSON string describing available backends
 * @return Duplicated backend name (CALLER MUST FREE) or NULL on failure
 */
char* select_backend(char* backends_data) {
    char* backend = NULL;

    cJSON* backends_data_cjson = cJSON_Parse(backends_data);
    if (!backends_data_cjson) {
        fprintf(stderr, "ERROR - Parsing backends data JSON failed in select_backend()!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* devices_cjson = cJSON_GetObjectItemCaseSensitive(backends_data_cjson, "devices");
    if (!devices_cjson || !devices_cjson->child) {
        fprintf(stderr, "ERROR - Parsing devices list failed in select_backend()!\n");
        goto cleanup_backends_data_cjson;
    }

    int least_busy_device_queue = INT_MAX;
    char* least_busy_device_name = NULL;

    for (cJSON* device_cjson = devices_cjson->child; device_cjson; device_cjson = device_cjson->next) {
        int device_jobs = 0;

        cJSON* device_jobs_cjson = cJSON_GetObjectItemCaseSensitive(device_cjson, "queue_length");
        if (cJSON_IsNumber(device_jobs_cjson)) {
            device_jobs = device_jobs_cjson->valueint;
        } else {
            fprintf(stderr, "ERROR - Parsing the queue length failed in select_backend()!\n");
            goto cleanup_backends_data_cjson;
        }
        
        if (0 <= device_jobs && device_jobs < least_busy_device_queue) {
            least_busy_device_queue = device_jobs;

            cJSON* device_name_cjson = cJSON_GetObjectItemCaseSensitive(device_cjson, "name");
            if (cJSON_IsString(device_name_cjson) && device_name_cjson->valuestring) {
                least_busy_device_name = device_name_cjson->valuestring;
            } else {
                fprintf(stderr, "ERROR - Parsing the device name failed in select_backend()!\n");
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

/**
 * @brief Build job submission payload
 *
 * Constructs the JSON payload to submit a sampling job for the provided
 * backend and OpenQASM program.
 *
 * @param backend Backend name to target
 * @param qasm OpenQASM program string
 * @return JSON payload string (CALLER MUST FREE) or NULL
 */
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

/**
 * @brief Submit a job to the IBM Quantum jobs endpoint
 *
 * Posts the provided JSON payload to the jobs endpoint and returns the raw
 * response body when successful.
 *
 * @param token_data Pointer to TOKEN_DATA with authentication token
 * @param crn Service CRN string
 * @param payload JSON payload to submit
 * @return Response body string on success (CALLER MUST FREE) or NULL on error
 */
char* submit_job(TOKEN_DATA* token_data, char* crn, char* payload) {
    char* response = NULL;

    char* token = copy_bearer_token(token_data);
    if (!token) {
        fprintf(stderr, "ERROR - Copying bearer token failed in submit_job()!\n");
        goto terminate;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR - cURL initialization failed in submit_job()!\n");
        goto cleanup_token;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed in submit_job()!\n");
        goto cleanup_curl;
    }

    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!token_header) {
        fprintf(stderr, "ERROR - Allocating memory for token header failed in submit_job()!\n");
        goto cleanup_rb;
    }

    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!crn_header) {
        fprintf(stderr, "ERROR - Allocating memory for CRN header failed in submit_job()!\n");
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
        fprintf(stderr, "ERROR - Header construction failed in submit_job()!\n");
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

    CURLcode response_code = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response_code != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Job submission failed in submit_job()!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response_code));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        if (rb.data && rb.size > 0) fprintf(stderr, "ERROR - Response Body: %s\n", rb.data);
        goto cleanup_headers;
    }

    response = strdup(rb.data);

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
    return response;
}

/**
 * @brief Parse job ID from job submission response
 *
 * Extracts the id field from the job submission JSON response.
 *
 * @param response JSON response string from job submission
 * @return Duplicated job id string (CALLER MUST FREE) or NULL on failure
 */
char* parse_job_id(char* response) {
    char* job_id = NULL;

    cJSON* response_cjson = cJSON_Parse(response);
    if (!response_cjson) {
        fprintf(stderr, "ERROR - Parsing job submission response JSON failed in parse_job_id()!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* job_id_cjson = cJSON_GetObjectItemCaseSensitive(response_cjson, "id");
    if (cJSON_IsString(job_id_cjson) && job_id_cjson->valuestring) {
        job_id = strdup(job_id_cjson->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing job ID from response failed in parse_job_id()!\n");
        goto cleanup_response_cjson;
    }

cleanup_response_cjson:
    cJSON_Delete(response_cjson);

terminate:
    return job_id;
}


/**
 * @brief High-level sender: select backend, submit job, return job id
 *
 * Waits for the authenticator to provide a token, selects a backend,
 * constructs the payload, submits the job and returns the job id.
 *
 * @param token_data Pointer to TOKEN_DATA used for authentication
 * @param crn Service CRN string
 * @param qasm OpenQASM program string to submit
 * @return Duplicated job id string (CALLER MUST FREE) or NULL on failure
 */
char* sender(TOKEN_DATA* token_data, char* crn, char* qasm) {
    pthread_mutex_lock(&token_data->lock);
    while (!token_data->token_received_bool) {
        pthread_cond_wait(&token_data->token_received_cond, &token_data->lock);
    }
    pthread_mutex_unlock(&token_data->lock);

    char* job_id = NULL;

    char* backends_data = get_backends_data(token_data, crn);
    if (!backends_data) {
        fprintf(stderr, "ERROR - Fetching backends data failed in sender()!\n");
        goto terminate;
    }

    char* backend = select_backend(backends_data);
    if (!backend) {
        fprintf(stderr, "ERROR - Selecting the backend device failed in sender()!\n");
        goto cleanup_backends_data;
    }

    char* payload = build_payload(backend, qasm);
    if (!payload) {
        fprintf(stderr, "ERROR - Building payload for job submission failed in sender()!\n");
        goto cleanup_backend;
    }

    char* response = submit_job(token_data, crn, payload);
    if (!response) {
        fprintf(stderr, "ERROR - Getting a response from job submission failed in sender()!\n");
        goto cleanup_payload;
    }

    job_id = parse_job_id(response);
    if (!job_id) {
        fprintf(stderr, "ERROR - Parsing the job ID failed in sender()!\n");
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "comm.h"
#include "auth.h"


char* get_bearer_token(TOKEN_DATA* token_data) {
    char* response_data = NULL;
    char* api_key = token_data->api_key;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        goto terminate;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed1\n");
        goto cleanup_curl;
    }

    char* escaped = curl_easy_escape(curl, api_key, 0);
    if (!escaped) {
        fprintf(stderr, "ERROR - API Key escaping failed!\n");
        goto cleanup_rb;
    }

    char* payload = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!payload) {
        fprintf(stderr, "ERROR - Allocating memory for payload failed!\n");
        goto cleanup_escaped;
    }
    snprintf(payload, BUFFER_NMEMB, "grant_type=urn:ibm:params:oauth:grant-type:apikey&apikey=%s", escaped);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    if (!headers) {
        fprintf(stderr, "ERROR - Content type appending failed!\n");
        goto cleanup_payload;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://iam.cloud.ibm.com/identity/token");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    CURLcode response = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Authentication request failed!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        goto cleanup_payload;
    }
    
    response_data = strdup(rb.data);

cleanup_payload:
    free(payload);

cleanup_escaped:
    curl_free(escaped);

cleanup_rb:
    free(rb.data);

cleanup_curl:
    curl_easy_cleanup(curl);

terminate:
    return response_data;
}

void update_bearer_token(TOKEN_DATA* token_data, char* token) {
    pthread_mutex_lock(&token_data->lock);
    free(token_data->token);
    token_data->token = strdup(token);
    pthread_mutex_unlock(&token_data->lock);

    return;
}

int parse_bearer_token(TOKEN_DATA* token_data, char* response) {
    int expiration_time = -1;

    cJSON* data_cjson = cJSON_Parse(response);
    if (!data_cjson) {
        fprintf(stderr, "ERROR - Parsing bearer token data JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* cjson_bearer_token = cJSON_GetObjectItemCaseSensitive(data_cjson, "access_token");
    if (cJSON_IsString(cjson_bearer_token) && cjson_bearer_token->valuestring) {
        update_bearer_token(token_data, cjson_bearer_token->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing bearer token failed!\n");
        goto cleanup_data_cjson;
    }

    cJSON* cjson_expiration_time = cJSON_GetObjectItemCaseSensitive(data_cjson, "expires_in");
    if (cJSON_IsNumber(cjson_expiration_time) && cjson_expiration_time->valueint > 0) {
        expiration_time = cjson_expiration_time->valueint;
    } else {
        fprintf(stderr, "ERROR - The expiration time is not valid!\n");
        goto cleanup_data_cjson;
    }

cleanup_data_cjson:
    cJSON_Delete(data_cjson);
    
terminate:
    return expiration_time;
}


void* authenticator(void* arg) {
    TOKEN_DATA* token_data = (TOKEN_DATA*)arg;
    long termination_status = EXIT_FAILURE;

    // Obtain the first token.

    char* response = get_bearer_token(token_data);
    if (!response) {
        fprintf(stderr, "ERROR - Obtaining bearer token failed!\n");
        goto terminate;
    }

    int expiration_time = parse_bearer_token(token_data, response);
    if (expiration_time < 0) {
        fprintf(stderr, "ERROR - Parsing bearer token failed!\n");
        goto cleanup_response;
    } else if (expiration_time < OFFSET_TIME) {
        fprintf(stderr, "ERROR - The given expiration time is less than the offset of %d seconds!\n", OFFSET_TIME);
        goto cleanup_response;
    }

    // Signal that the first token was received to start sending the job.

    signal_token_received(token_data);

    // Check if the expiration time has passed and the termination signal has activated.
    // If the expiration time has passed, get a new bearer token.
    // If the termination has been announced, terminate this thread to join the main thread.

    while (true) {
        pthread_mutex_lock(&token_data->lock);

        // Terminate this thread once the job termination is announced.

        if (token_data->job_terminated_bool) {
            pthread_mutex_unlock(&token_data->lock);
            break;
        }

        // Compute the wake up time.

        struct timespec time_spec;
        struct timeval now;
        gettimeofday(&now, NULL);

        time_spec.tv_sec = now.tv_sec + (expiration_time - OFFSET_TIME);
        time_spec.tv_nsec = now.tv_usec * 1000;

        // Go to sleep until the token expires or the job termination announced.
        // The function `pthread_cond_timedwait()` requires the mutex to be locked and locks the mutex right before the return.
        
        int wait_result = pthread_cond_timedwait(&token_data->job_terminated_cond, &token_data->lock, &time_spec);

        // Terminate this thread once the job termination is announced.

        if (token_data->job_terminated_bool) {
            pthread_mutex_unlock(&token_data->lock);
            break;
        }

        // Get a new token since the previous token just got expired.

        if (wait_result == ETIMEDOUT) {
            pthread_mutex_unlock(&token_data->lock);
            response = get_bearer_token(token_data); // FIXME: Potential memory leak.
            expiration_time = parse_bearer_token(token_data, response);
            pthread_mutex_lock(&token_data->lock);
        }

        pthread_mutex_unlock(&token_data->lock);
    }

    termination_status = EXIT_SUCCESS;

cleanup_response:
    free(response);

terminate:
    pthread_exit((void*)termination_status);

    return NULL;
}

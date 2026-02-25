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


/**
 * @brief Obtain a bearer token from IBM IAM
 *
 * Calls the IAM token endpoint using the API key stored in token_data and
 * accumulates the HTTP response body.
 *
 * @param token_data Pointer to TOKEN_DATA containing the API key and token
 * @return Newly allocated response body string on success (CALLER MUST FREE),
 *         or NULL on failure
 */
char* get_bearer_token(TOKEN_DATA* token_data) {
    char* response = NULL;
    char* key = token_data->key;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR - cURL initialization failed in get_bearer_token()!\n");
        goto terminate;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed in get_bearer_token()!\n");
        goto cleanup_curl;
    }

    char* escaped = curl_easy_escape(curl, key, 0);
    if (!escaped) {
        fprintf(stderr, "ERROR - API key escaping failed in get_bearer_token()!\n");
        goto cleanup_rb;
    }

    char* payload = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!payload) {
        fprintf(stderr, "ERROR - Allocating memory for payload failed in get_bearer_token()!\n");
        goto cleanup_escaped;
    }
    snprintf(payload, BUFFER_NMEMB, "grant_type=urn:ibm:params:oauth:grant-type:apikey&apikey=%s", escaped);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    if (!headers) {
        fprintf(stderr, "ERROR - Header construction failed in get_bearer_token()!\n");
        goto cleanup_payload;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://iam.cloud.ibm.com/identity/token");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    CURLcode response_code = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (response_code != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Authentication request failed in get_bearer_token()!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response_code));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        goto cleanup_headers;
    }
    
    response = strdup(rb.data);

cleanup_headers:
    curl_slist_free_all(headers);

cleanup_payload:
    free(payload);

cleanup_escaped:
    curl_free(escaped);

cleanup_rb:
    free(rb.data);

cleanup_curl:
    curl_easy_cleanup(curl);

terminate:
    return response;
}

/**
 * @brief Update the stored bearer token (thread-safe)
 *
 * Replace the token field in token_data with a copy of the provided token.
 * The operation is protected by the token_data mutex.
 *
 * @param token_data Pointer to TOKEN_DATA to update
 * @param token New token string to store (function copies it)
 */
void update_bearer_token(TOKEN_DATA* token_data, char* token) {
    pthread_mutex_lock(&token_data->lock);
    free(token_data->token);
    token_data->token = strdup(token);
    pthread_mutex_unlock(&token_data->lock);

    return;
}

/**
 * @brief Parse IAM token JSON and update token data
 *
 * Extracts access_token and expires_in from the IAM JSON response and
 * updates token_data with the token. On success returns the expiration time
 * in seconds.
 *
 * @param token_data Pointer to TOKEN_DATA to update
 * @param response JSON response string from IAM
 * @return Expiration time in seconds on success, or -1 on failure
 */
int parse_bearer_token(TOKEN_DATA* token_data, char* response) {
    int expiration_time = -1;

    cJSON* data_cjson = cJSON_Parse(response);
    if (!data_cjson) {
        fprintf(stderr, "ERROR - Parsing bearer token JSON failed in parse_bearer_token()!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* bearer_token_cjson = cJSON_GetObjectItemCaseSensitive(data_cjson, "access_token");
    if (cJSON_IsString(bearer_token_cjson) && bearer_token_cjson->valuestring) {
        update_bearer_token(token_data, bearer_token_cjson->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing bearer token failed in parse_bearer_token()!\n");
        goto cleanup_data_cjson;
    }

    cJSON* expiration_time_cjson = cJSON_GetObjectItemCaseSensitive(data_cjson, "expires_in");
    if (cJSON_IsNumber(expiration_time_cjson) && expiration_time_cjson->valueint > 0) {
        expiration_time = expiration_time_cjson->valueint;
    } else {
        fprintf(stderr, "ERROR - The expiration time is not valid in parse_bearer_token()!\n");
        goto cleanup_data_cjson;
    }

cleanup_data_cjson:
    cJSON_Delete(data_cjson);
    
terminate:
    return expiration_time;
}


/**
 * @brief Authenticator thread that refreshes the bearer token
 *
 * Entry point for the authenticator thread. Obtains the initial bearer
 * token, signals waiting threads, and refreshes the token before expiration
 * in a loop until job termination is requested.
 *
 * @param arg Pointer to TOKEN_DATA passed to the thread
 * @return Thread exit value (returns NULL, pthread_exit used)
 */
void* authenticator(void* arg) {
    TOKEN_DATA* token_data = (TOKEN_DATA*)arg;
    long termination_status = EXIT_FAILURE;

    // Obtain the first token.

    char* response = get_bearer_token(token_data);
    if (!response) {
        fprintf(stderr, "ERROR - Obtaining bearer token failed in authenticator()!\n");
        goto terminate;
    }

    int expiration_time = parse_bearer_token(token_data, response);
    if (expiration_time < 0) {
        fprintf(stderr, "ERROR - Parsing bearer token failed in authenticator()!\n");
        goto cleanup_response;
    } else if (expiration_time < OFFSET_TIME) {
        fprintf(stderr, "ERROR - The given expiration time (%d seconds) is less than the offset (%d seconds) in authenticator()!\n", expiration_time, OFFSET_TIME);
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
        // The function pthread_cond_timedwait() requires the mutex to be locked and locks the mutex right before the return.
        
        int wait_result = pthread_cond_timedwait(&token_data->job_terminated_cond, &token_data->lock, &time_spec);

        // Terminate this thread once the job termination is announced.

        if (token_data->job_terminated_bool) {
            pthread_mutex_unlock(&token_data->lock);
            break;
        }

        // Get a new token since the previous token just got expired.

        if (wait_result == ETIMEDOUT) {
            pthread_mutex_unlock(&token_data->lock);
            char* response = get_bearer_token(token_data);
            expiration_time = parse_bearer_token(token_data, response);
            pthread_mutex_lock(&token_data->lock);
            free(response);
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

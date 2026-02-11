#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "comm.h"
#include "auth.h"

void update_bearer_token(TOKEN_DATA* token_data, const char* bearer_token) {
    pthread_mutex_lock(&token_data->lock);

    size_t token_length = strlen(bearer_token);
    token_data->token = (char*)realloc(token_data->token, sizeof(char)*(token_length+1));
    strcpy(token_data->token, bearer_token);

    pthread_mutex_unlock(&token_data->lock);

    return;
}

// Returns expiration time.
// TODO: Refactoring may be required.
int get_bearer_token(TOKEN_DATA* token_data) {
    const char* api_key = token_data->api_key;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        return -1;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    struct curl_slist* headers = NULL; 

    char* escaped = curl_easy_escape(curl, api_key, 0);
    if (!escaped) {
        fprintf(stderr, "ERROR - API Key escaping failed!\n");
        free(rb.data);
        curl_easy_cleanup(curl);
        return -1;
    }

    // TODO: Implement overflow checking.
    // TODO: Even better, implement dynamic size allocation.
    char* payload = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    snprintf(payload, BUFFER_NMEMB, "grant_type=urn:ibm:params:oauth:grant-type:apikey&apikey=%s", escaped);

    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    if (!headers) {
        fprintf(stderr, "ERROR - Content type appending failed!\n");
        free(rb.data);
        curl_easy_cleanup(curl);
        curl_free(escaped);
        return -1;
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
        free(rb.data);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_free(escaped);
        return -1;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_free(escaped);
    free(payload);

    cJSON* cjson_data = cJSON_Parse(rb.data);
    if (!cjson_data) {
        fprintf(stderr, "ERROR - Parsing bearer token data JSON failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) {
            fprintf(stderr, "ERROR - %s\n", error);
        }
        free(rb.data);
        cJSON_Delete(cjson_data);
        return -1;
    }

    int expiration_time = 0;
    cJSON* cjson_expiration_time = cJSON_GetObjectItemCaseSensitive(cjson_data, "expires_in");
    if (cJSON_IsNumber(cjson_expiration_time) && cjson_expiration_time->valueint > 0) {
        expiration_time = cjson_expiration_time->valueint;
    } else {
        fprintf(stderr, "ERROR - The expiration time is not valid!\n");
        free(rb.data);
        cJSON_Delete(cjson_data);
        return -1;
    }

    cJSON* cjson_bearer_token = cJSON_GetObjectItemCaseSensitive(cjson_data, "access_token");
    if (cJSON_IsString(cjson_bearer_token) && cjson_bearer_token->valuestring) {
        update_bearer_token(token_data, (const char*)cjson_bearer_token->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing bearer token failed!\n");
        free(rb.data);
        cJSON_Delete(cjson_data);
        return -1;
    }

    free(rb.data);
    cJSON_Delete(cjson_data);
    
    return expiration_time;
}

void* start_authenticator(void* arg) {
    TOKEN_DATA* token_data = (TOKEN_DATA*)arg;

    int expiration_time = get_bearer_token(token_data);
    if (expiration_time < 0) {
        fprintf(stderr, "ERROR - Obtaining bearer token failed!\n");
        return NULL;
    }

    printf("Bearer Token: %s\n", token_data->token);

    return NULL;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "auth.h"

void update_bearer_token() {

}

char* get_bearer_token(const char* api_key) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        return NULL;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    struct curl_slist* headers = NULL; 

    char* escaped = curl_easy_escape(curl, api_key, 0);
    if (!escaped) {
        fprintf(stderr, "ERROR - API Key escaping failed!\n");
        free(rb.data);
        curl_easy_cleanup(curl);
        return NULL;
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
        return NULL;
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
        return NULL;
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
        return NULL;
    }

    char* bearer_token = (char*)calloc(rb.size+1, sizeof(char));
    cJSON* cjson_bearer_token = cJSON_GetObjectItemCaseSensitive(cjson_data, "access_token");
    if (cJSON_IsString(cjson_bearer_token) && cjson_bearer_token->valuestring) {
        strcpy(bearer_token, cjson_bearer_token->valuestring);
    } else {
        fprintf(stderr, "ERROR - Parsing bearer token failed!\n");
        free(rb.data);
        cJSON_Delete(cjson_data);
        return NULL;
    }

    free(rb.data);
    cJSON_Delete(cjson_data);
    
    return bearer_token;
}

void start_authenticator(const char* api_key) {
    pthread_t auth_thread;

    pthread_create(&auth_thread, NULL, get_bearer_token, NULL);

    return;
}

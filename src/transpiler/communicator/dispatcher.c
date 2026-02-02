#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "dispatcher.h"


size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    RESPONSEBUFFER *rb = userp;

    char *temp = realloc(rb->data, rb->size + real_size + 1);
    if (!temp) return 0;

    rb->data = temp;
    memcpy(rb->data + rb->size, contents, real_size);
    rb->size += real_size;
    rb->data[rb->size] = '\0';

    return real_size;
}


char* ibm_authentication(const char* api_key) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        return NULL;
    }

    RESPONSEBUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/x-www-form-urlencoded");
    if (!headers) {
        fprintf(stderr, "ERROR - Content type appending failed!\n");
        free(rb.data);
        rb.data = NULL;
        curl_easy_cleanup(curl);
        return NULL;
    }

    char* escaped = curl_easy_escape(curl, api_key, 0);
    if (!escaped) {
        fprintf(stderr, "ERROR - API Key escaping failed!\n");
        free(rb.data);
        rb.data = NULL;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return NULL;
    }

    char payload[512];
    snprintf(payload, sizeof(payload), "grant_type=urn:ibm:params:oauth:grant-type:apikey&apikey=%s", escaped);

    curl_easy_setopt(curl, CURLOPT_URL, "https://iam.cloud.ibm.com/identity/token");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode response = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &http_code);
    if (response != CURLE_OK || http_code >= 400) {
        fprintf(stderr, "ERROR - Authentication request failed!\n");
        fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
        fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
        free(rb.data);
        rb.data = NULL;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_free(escaped);
        return NULL;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_free(escaped);
    
    return rb.data;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "comm.h"
#include "receiver.h"


char* receiver(TOKEN_DATA* token_data, char* crn, char* job_id) {
    char* token = copy_bearer_token(token_data);

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: cURL initialization failed!\n");
        free(token);
        return NULL;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    struct curl_slist* headers = NULL;

    char* url = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));

    snprintf(url, BUFFER_NMEMB, "https://quantum.cloud.ibm.com/api/v1/jobs/%s/results", job_id);
    snprintf(token_header, BUFFER_NMEMB, "Authorization: Bearer %s", token);
    snprintf(crn_header, BUFFER_NMEMB, "Service-CRN: %s", crn);

    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, token_header);
    headers = curl_slist_append(headers, crn_header);
    headers = curl_slist_append(headers, "IBM-API-Version: 2026-02-01");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    /* Enable verbose libcurl logging when QC_DEBUG=1 in environment */
    const char* qc_debug = getenv("QC_DEBUG");
    if (qc_debug && strcmp(qc_debug, "1") == 0) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    long http_code = 0;
    do {
        CURLcode response = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (response != CURLE_OK || http_code > 400) {
            fprintf(stderr, "ERROR - Getting job result failed!\n");
            fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
            fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
            free(token);
            free(rb.data);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            free(url);
            free(token_header);
            free(crn_header);
            return NULL;
        } else if (http_code == 400) { // TODO: Check if the code is 1234.
            if (rb.data && rb.size > 0) {
                fprintf(stderr, "ERROR - Response Body: %s\n", rb.data);
                free(rb.data);
                rb.data = calloc(1, sizeof(char));
                rb.size = 0;
            }
        }

        sleep(WAIT_TIME);
    } while (http_code == 400);

    signal_job_terminated(token_data);

    free(token);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(url);
    free(token_header);
    free(crn_header);

    return rb.data;
}

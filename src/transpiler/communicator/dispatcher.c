#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#include "dispatcher.h"


static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {

}

static char* send_http_request(const char* url, const char* method, const char* json_payload, const char* auth_token) {
    CURL *curl;
    CURLcode res;
    RESPONSEBUF chunk;
    chunk.memory = malloc(1); 
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        if (auth_token) {
            char auth_header[512];
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", auth_token);
            headers = curl_slist_append(headers, auth_header);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        }

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            chunk.memory = NULL;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return chunk.memory;
}

static char* extract_json_value(const char* json, const char* key) {

}

static char* get_access_token() {
    char* api_key = getenv("IBM_QUANTUM_KEY");
    if (!api_key) {
        fprintf(stderr, "ERROR: Environment variable IBM_QUANTUM_KEY not set.\n");
        return NULL;
    }

    char payload[1024];
    snprintf(payload, sizeof(payload), "{\"apiToken\": \"%s\"}", api_key);
    
    char* response = send_http_request(IBM_AUTH_URL, "POST", payload, NULL);
    if (!response) return NULL;

    char* token = extract_json_value(response, "id");
    free(response);
    return token;
}

static char* submit_job(const char* token, const char* qasm) {
    
}

static char* wait_for_result(const char* token, const char* job_id) {
    
}


char* run_quantum_circuit(const char* qasm_code) {
    char* token = get_access_token();
    if (!token) return strdup("Error: Auth Failed");

    char* job_id = submit_job(token, qasm_code);
    if (!job_id) {
        free(token);
        return strdup("Error: Submission Failed");
    }

    char* final_json = wait_for_result(token, job_id);
    
    free(token);
    free(job_id);

    if (final_json) {
        // Just return raw JSON for now (or parse 'quasi_dists' here)
        return final_json; 
    }

    return strdup("Error: No Result");
}

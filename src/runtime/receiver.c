#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "comm.h"
#include "receiver.h"


bool check_code(char* response) {
    bool is_code_1234 = false;

    cJSON* result_cjson = cJSON_Parse(response);
    if (!result_cjson) {
        fprintf(stderr, "ERROR - Parsing response packet from the backend failed!\n");
        const char* error = cJSON_GetErrorPtr();
        if (error) fprintf(stderr, "ERROR - %s\n", error);
        goto terminate;
    }

    cJSON* first_error_cjson = NULL;
    cJSON* errors_cjson = cJSON_GetObjectItemCaseSensitive(result_cjson, "errors");
    if (cJSON_IsArray(errors_cjson)) {
        first_error_cjson = errors_cjson->child;
    } else {
        fprintf(stderr, "ERROR - The errors part does not contain an error array!\n");
        goto cleanup_result_cjson;
    }

    cJSON* code_cjson = cJSON_GetObjectItemCaseSensitive(first_error_cjson, "code");
    if (cJSON_IsNumber(code_cjson) && code_cjson->valueint == 1234) {
        is_code_1234 = true;
    } else {
        fprintf(stderr, "ERROR - API returned error code: %d\n", code_cjson->valueint);
        cJSON* error_msg = cJSON_GetObjectItemCaseSensitive(first_error_cjson, "message");
        if (cJSON_IsString(error_msg) && error_msg->valuestring) {
            fprintf(stderr, "ERROR - Message: %s\n", error_msg->valuestring);
        }
    }

cleanup_result_cjson:
    cJSON_Delete(result_cjson);

terminate:
    return is_code_1234;
}

char* get_job_result(TOKEN_DATA* token_data, char* crn, char* job_id) {
    char* job_result = NULL;

    char* token = copy_bearer_token(token_data);
    if (!token) {
        fprintf(stderr, "ERROR - Copying bearer token failed!\n");
        goto terminate;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR - cURL initialization failed!\n");
        goto cleanup_token;
    }

    RESPONSE_BUFFER rb = {(char*)calloc(1, sizeof(char)), 0};
    if (!rb.data) {
        fprintf(stderr, "ERROR - Allocating memory for response buffer failed!\n");
        goto cleanup_curl;
    }

    char* url = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!url) {
        fprintf(stderr, "ERROR- Allocating memory for URL failed!\n");
        goto cleanup_rb;
    }

    char* token_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!token_header) {
        fprintf(stderr, "ERROR - Allocating memory for token header failed!\n");
        goto cleanup_url;
    }

    char* crn_header = (char*)calloc(BUFFER_NMEMB, sizeof(char));
    if (!crn_header) {
        fprintf(stderr, "ERROR - Allocating memory for CRN header failed!\n");
        goto cleanup_token_header;
    }

    snprintf(url, BUFFER_NMEMB, "https://quantum.cloud.ibm.com/api/v1/jobs/%s/results", job_id);
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

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_NAME);

    while (true) {
        CURLcode response = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code == 400 && check_code(rb.data)) {
            rb.data = (char*)realloc(rb.data, 1*sizeof(char));
            rb.size = 0;
            sleep(REFRESH_TIME);
            continue;
        }

        if (response != CURLE_OK || http_code >= 400) {
            fprintf(stderr, "ERROR - Getting job result failed!\n");
            fprintf(stderr, "ERROR - cURL Error: %s\n", curl_easy_strerror(response));
            fprintf(stderr, "ERROR - HTTP Code: %ld\n", http_code);
            goto cleanup_headers;
        }
        
        break;
    }

    job_result = strdup(rb.data);

cleanup_headers:
    curl_slist_free_all(headers);

cleanup_crn_header:
    free(crn_header);

cleanup_token_header:
    free(token_header);

cleanup_url:
    free(url);

cleanup_rb:
    free(rb.data);

cleanup_curl:
    curl_easy_cleanup(curl);

cleanup_token:
    free(token);

terminate:
    return job_result;
}

char* parse_job_result(char* result) {
    char* result_sample = NULL;

    cJSON* result_cjson = cJSON_Parse(result);
    if (!result_cjson) {
        fprintf(stderr, "ERROR - Parsing result JSON failed!\n");
        goto terminate;
    }

    // Navigate to results[0].data.meas.samples.

    cJSON* results_array = cJSON_GetObjectItemCaseSensitive(result_cjson, "results");
    if (!results_array || !results_array->child) {
        fprintf(stderr, "ERROR - No results array found!\n");
        goto cleanup_result_cjson;
    }

    cJSON* first_result = results_array->child;
    cJSON* data = cJSON_GetObjectItemCaseSensitive(first_result, "data");
    if (!data) {
        fprintf(stderr, "ERROR - No data field in result!\n");
        goto cleanup_result_cjson;
    }

    cJSON* meas = cJSON_GetObjectItemCaseSensitive(data, "meas");
    if (!meas) {
        fprintf(stderr, "ERROR - No meas field in data!\n");
        goto cleanup_result_cjson;
    }

    cJSON* samples_array = cJSON_GetObjectItemCaseSensitive(meas, "samples");
    if (!samples_array || !samples_array->child) {
        fprintf(stderr, "ERROR - No samples array found!\n");
        goto cleanup_result_cjson;
    }

    // Count frequencies of each sample.


    int max_unique_samples = 1 << 4; // This depends on the number of qubits of the circuit.
    SAMPLE_COUNT* sample_counts = (SAMPLE_COUNT*)calloc(max_unique_samples, sizeof(SAMPLE_COUNT));
    int unique_count = 0;

    // Collect all unique samples and count.

    for (cJSON* sample_item = samples_array->child; sample_item; sample_item = sample_item->next) {
        if (!cJSON_IsString(sample_item)) continue;

        const char* sample_str = sample_item->valuestring;
        
        // Find or add this sample to our tracking array.

        int found_index = -1;
        for (int i = 0; i < unique_count; i++) {
            if (strcmp(sample_counts[i].sample, sample_str) == 0) {
                found_index = i;
                break;
            }
        }

        if (found_index >= 0) {
            sample_counts[found_index].count++;
        } else {
            if (unique_count >= max_unique_samples) {
                fprintf(stderr, "ERROR - Too many unique samples!\n");
                goto cleanup_sample_counts;
            }
            sample_counts[unique_count].sample = strdup(sample_str);
            sample_counts[unique_count].count = 1;
            unique_count++;
        }
    }

    // Find the sample with highest frequency.

    int max_count = 0;
    char* most_frequent = NULL;
    for (int i = 0; i < unique_count; i++) {
        if (sample_counts[i].count > max_count) {
            most_frequent = sample_counts[i].sample;
            max_count = sample_counts[i].count;
        }
    }

    result_sample = strdup(most_frequent);

cleanup_sample_counts:
    for (int i = 0; i < unique_count; i++) {
        free(sample_counts[i].sample);
    }
    free(sample_counts);

cleanup_result_cjson:
    cJSON_Delete(result_cjson);

terminate:
    return result_sample;
}

char* convert_job_result(char* sample) {
    char* binary_str = NULL;

    // Parse the hexadecimal value.

    unsigned long hex_value = 0;
    if (sscanf(sample, "0x%lx", &hex_value) != 1) {
        fprintf(stderr, "ERROR - Failed to parse hex sample: %s\n", sample);
        return NULL;
    }

    // Determine number of bits.

    int num_bits = 0;
    unsigned long temp = hex_value;
    while (temp > 0) {
        num_bits++;
        temp >>= 1;
    }
    
    if (num_bits == 0) {
        num_bits = 1;
    }

    // Allocate binary string.

    binary_str = (char*)calloc(num_bits+1, sizeof(char));
    if (!binary_str) {
        fprintf(stderr, "ERROR - Memory allocation failed for binary string!\n");
        goto terminate;
    }

    // Convert to binary (most significant bit first).

    for (int i = num_bits-1; i >= 0; i--) {
        binary_str[num_bits-1-i] = ((hex_value >> i) & 1) ? '1' : '0';
    }
    binary_str[num_bits] = '\0';

terminate:
    return binary_str;
}


char* receiver(TOKEN_DATA* token_data, char* crn, char* job_id) {
    char* binary_string = NULL;

    char* response = get_job_result(token_data, crn, job_id);
    if (!response) {
        fprintf(stderr, "ERROR - Getting the job result from the backend failed!\n");
        goto terminate;
    }

    char* sample = parse_job_result(response);
    if (!sample) {
        fprintf(stderr, "ERROR - Result parsing failed!\n");
        goto cleanup_response;
    }

    binary_string = convert_job_result(sample);
    if (!binary_string) {
        fprintf(stderr, "ERROR - Result bit string conversion failed1\n");
        goto cleanup_sample;
    }

cleanup_sample:
    free(sample);

cleanup_response:
    free(response);

terminate:
    return binary_string;
}

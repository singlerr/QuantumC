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


char* get_result(TOKEN_DATA* token_data, char* crn, char* job_id) {
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

    while (true) {
        CURLcode response = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code == 400) {
            cJSON* result_cjson = cJSON_Parse(rb.data);
            if (result_cjson) {
                cJSON* errors_array = cJSON_GetObjectItemCaseSensitive(result_cjson, "errors");
                if (errors_array && errors_array->child) {
                    cJSON* first_error = errors_array->child;
                    cJSON* error_code = cJSON_GetObjectItemCaseSensitive(first_error, "code");
                    
                    if (cJSON_IsNumber(error_code) && error_code->valueint == 1234) {
                        fprintf(stderr, "INFO - Job not yet in terminal state (code 1234). Retrying in %d seconds...\n", REFRESH_TIME);
                        cJSON_Delete(result_cjson);
                        
                        // Clear buffer for next request
                        rb.data = (char*)realloc(rb.data, 1*sizeof(char));
                        rb.size = 0;
                        
                        sleep(REFRESH_TIME);
                        continue;  // Retry the loop
                    } else {
                        fprintf(stderr, "ERROR - API returned error code: %d\n", error_code->valueint);
                        cJSON* error_msg = cJSON_GetObjectItemCaseSensitive(first_error, "message");
                        if (cJSON_IsString(error_msg) && error_msg->valuestring) {
                            fprintf(stderr, "ERROR - Message: %s\n", error_msg->valuestring);
                        }
                    }
                }
                cJSON_Delete(result_cjson);
            }
            free(token);
            free(rb.data);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            free(url);
            free(token_header);
            free(crn_header);
            return NULL;
        }

        if (response != CURLE_OK || http_code >= 400) {
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
        }
        
        break;
    }

    free(token);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(url);
    free(token_header);
    free(crn_header);

    return rb.data;
}

char* parse_result(char* result) {
    cJSON* result_cjson = cJSON_Parse(result);
    if (!result_cjson) {
        fprintf(stderr, "ERROR - Parsing result JSON failed!\n");
        return NULL;
    }

    // Navigate to results[0].data.meas.samples.

    cJSON* results_array = cJSON_GetObjectItemCaseSensitive(result_cjson, "results");
    if (!results_array || !results_array->child) {
        fprintf(stderr, "ERROR - No results array found!\n");
        cJSON_Delete(result_cjson);
        return NULL;
    }

    cJSON* first_result = results_array->child;
    cJSON* data = cJSON_GetObjectItemCaseSensitive(first_result, "data");
    if (!data) {
        fprintf(stderr, "ERROR - No data field in result!\n");
        cJSON_Delete(result_cjson);
        return NULL;
    }

    cJSON* meas = cJSON_GetObjectItemCaseSensitive(data, "meas");
    if (!meas) {
        fprintf(stderr, "ERROR - No meas field in data!\n");
        cJSON_Delete(result_cjson);
        return NULL;
    }

    cJSON* samples_array = cJSON_GetObjectItemCaseSensitive(meas, "samples");
    if (!samples_array || !samples_array->child) {
        fprintf(stderr, "ERROR - No samples array found!\n");
        cJSON_Delete(result_cjson);
        return NULL;
    }

    // Count frequencies of each sample
    // We'll iterate twice: once to find unique samples, once to count
    typedef struct {
        char* sample;
        int count;
    } SampleCount;

    int max_unique_samples = 16;  // Should be enough for most cases (2^4 outcomes)
    SampleCount* sample_counts = (SampleCount*)calloc(max_unique_samples, sizeof(SampleCount));
    int unique_count = 0;

    // First pass: collect all unique samples and count
    for (cJSON* sample_item = samples_array->child; sample_item; sample_item = sample_item->next) {
        if (!cJSON_IsString(sample_item)) continue;

        const char* sample_str = sample_item->valuestring;
        
        // Find or add this sample to our tracking array
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
                for (int i = 0; i < unique_count; i++) {
                    free(sample_counts[i].sample);
                }
                free(sample_counts);
                cJSON_Delete(result_cjson);
                return NULL;
            }
            sample_counts[unique_count].sample = strdup(sample_str);
            sample_counts[unique_count].count = 1;
            unique_count++;
        }
    }

    // Find the sample with highest frequency
    int max_count = 0;
    char* most_frequent = NULL;
    for (int i = 0; i < unique_count; i++) {
        if (sample_counts[i].count > max_count) {
            max_count = sample_counts[i].count;
            most_frequent = sample_counts[i].sample;
        }
    }

    char* result_sample = NULL;
    if (most_frequent) {
        result_sample = strdup(most_frequent);
    }

    // Cleanup
    for (int i = 0; i < unique_count; i++) {
        free(sample_counts[i].sample);
    }
    free(sample_counts);
    cJSON_Delete(result_cjson);

    return result_sample;
}

char* convert_result(char* sample) { 
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

    char* binary_string = (char*)calloc(num_bits+1, sizeof(char));
    if (!binary_string) {
        fprintf(stderr, "ERROR - Memory allocation failed for binary string!\n");
        return NULL;
    }

    // Convert to binary (most significant bit first).

    for (int i = num_bits-1; i >= 0; i--) {
        binary_string[num_bits - 1 - i] = ((hex_value >> i) & 1) ? '1' : '0';
    }
    binary_string[num_bits] = '\0';

    return binary_string;
}


char* receiver(TOKEN_DATA* token_data, char* crn, char* job_id) {
    char* result_json = get_result(token_data, crn, job_id);
    if (!result_json) {
        fprintf(stderr, "ERROR - Getting the job result from the backend failed!\n");
        return NULL;
    }

    char* most_frequent_sample = parse_result(result_json);
    if (!most_frequent_sample) {
        fprintf(stderr, "ERROR - Result parsing failed!\n");
        free(result_json);
        return NULL;
    }

    char* binary_string = convert_result(most_frequent_sample);
    if (!binary_string) {
        fprintf(stderr, "ERROR - Result bit string conversion failed1\n");
        free(result_json);
        free(most_frequent_sample);
        return NULL;
    }

    return binary_string;
}

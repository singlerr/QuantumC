#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "dispatcher.h"

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;
    struct ResponseBuffer *mem = (struct ResponseBuffer *)userp;

    char *ptr = realloc(mem->data, mem->size + real_size + 1);
    if (!ptr)
    {
        printf("ERROR - Memory allocation failed.\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->data[mem->size] = '\0';
    q return real_size;
}

char *send_post_request(const char *url, const char *json_payload, const char *auth_token)
{
    CURL *curl;
    CURLcode res;

    struct ResponseBuffer response_chunk;
    response_chunk.data = (char *)calloc(1, sizeof(char));
    response_chunk.size = 0;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (auth_token != NULL)
        {
            char auth_header[256];
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", auth_token);
            headers = curl_slist_append(headers, auth_header);
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "Request Failed: %s\n", curl_easy_strerror(res));
            free(response_chunk.data);
            response_chunk.data = NULL;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return response_chunk.data;
}

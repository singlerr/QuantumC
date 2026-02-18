#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "comm.h"


size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    RESPONSE_BUFFER* rb = userp;

    char* temp = realloc(rb->data, rb->size+real_size+1);
    if (!temp) return 0;

    rb->data = temp;
    memcpy(rb->data+rb->size, contents, real_size);
    rb->size += real_size;
    rb->data[rb->size] = '\0';

    return real_size;
}


void initialize_token_data(TOKEN_DATA* token_data, char* api_key) {
    token_data->api_key = strdup(api_key);
    token_data->token = NULL;
    token_data->token_received_bool = false;
    token_data->job_terminated_bool = false;
    pthread_cond_init(&token_data->token_received_cond, NULL);
    pthread_cond_init(&token_data->job_terminated_cond, NULL);
    pthread_mutex_init(&token_data->lock, NULL);

    return;
}

void destroy_token_data(TOKEN_DATA* token_data) {
    free(token_data->api_key);
    free(token_data->token);
    pthread_cond_destroy(&token_data->token_received_cond);
    pthread_cond_destroy(&token_data->job_terminated_cond);
    pthread_mutex_destroy(&token_data->lock);
    free(token_data);

    return;
}


void signal_token_received(TOKEN_DATA* token_data) {
    pthread_mutex_lock(&token_data->lock);
    token_data->token_received_bool = true;
    pthread_mutex_unlock(&token_data->lock);
    pthread_cond_signal(&token_data->token_received_cond);

    return;
}

void signal_job_terminated(TOKEN_DATA* token_data) {
    pthread_mutex_lock(&token_data->lock);
    token_data->job_terminated_bool = true;
    pthread_mutex_unlock(&token_data->lock);
    pthread_cond_signal(&token_data->job_terminated_cond);

    return;
}


char* copy_bearer_token(TOKEN_DATA* token_data) {
    char* token = token_data->token;
    if (!token) {
        fprintf(stderr, "ERROR - The bearer token is not valid to copy!\n");
        return NULL;
    }

    pthread_mutex_lock(&token_data->lock);
    char* copy = strdup(token);
    pthread_mutex_unlock(&token_data->lock);

    return copy;
}

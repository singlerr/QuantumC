#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include "comm.h"


/**
 * @brief libcurl write callback to accumulate response data
 *
 * Appends incoming data to a RESPONSE_BUFFER provided in userp.
 *
 * @param contents Pointer to received data
 * @param size Size of each item
 * @param nmemb Number of items
 * @param userp Pointer to RESPONSE_BUFFER used to store data
 * @return Number of bytes written, or 0 on failure
 */
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


/**
 * @brief Initialize TOKEN_DATA and synchronization primitives
 *
 * Copies the provided API key into token_data and initializes condition
 * variables and mutex used by the runtime.
 *
 * @param token_data Pointer to TOKEN_DATA to initialize
 * @param key API key string to copy
 */
void initialize_token_data(TOKEN_DATA* token_data, char* key) {
    token_data->key = strdup(key);
    token_data->token = NULL;
    token_data->token_received_bool = false;
    token_data->job_terminated_bool = false;
    pthread_cond_init(&token_data->token_received_cond, NULL);
    pthread_cond_init(&token_data->job_terminated_cond, NULL);
    pthread_mutex_init(&token_data->lock, NULL);

    return;
}

/**
 * @brief Destroy TOKEN_DATA and free associated resources
 *
 * Frees memory held by token_data, destroys condition variables and mutex,
 * and frees the token_data structure itself.
 *
 * @param token_data Pointer to TOKEN_DATA to destroy
 */
void destroy_token_data(TOKEN_DATA* token_data) {
    free(token_data->key);
    free(token_data->token);
    pthread_cond_destroy(&token_data->token_received_cond);
    pthread_cond_destroy(&token_data->job_terminated_cond);
    pthread_mutex_destroy(&token_data->lock);
    free(token_data);

    return;
}


/**
 * @brief Signal that the initial token has been received
 *
 * Sets the token_received flag and signals the associated condition
 * variable so waiting threads can proceed.
 *
 * @param token_data Pointer to TOKEN_DATA to update
 */
void signal_token_received(TOKEN_DATA* token_data) {
    pthread_mutex_lock(&token_data->lock);
    token_data->token_received_bool = true;
    pthread_mutex_unlock(&token_data->lock);
    pthread_cond_signal(&token_data->token_received_cond);

    return;
}

/**
 * @brief Signal that the job has been terminated
 *
 * Sets the job_terminated flag and signals the associated condition
 * variable so waiting threads can exit.
 *
 * @param token_data Pointer to TOKEN_DATA to update
 */
void signal_job_terminated(TOKEN_DATA* token_data) {
    pthread_mutex_lock(&token_data->lock);
    token_data->job_terminated_bool = true;
    pthread_mutex_unlock(&token_data->lock);
    pthread_cond_signal(&token_data->job_terminated_cond);

    return;
}


/**
 * @brief Return a thread-safe copy of the bearer token
 *
 * Duplicates the token string inside token_data while holding the mutex.
 *
 * @param token_data Pointer to TOKEN_DATA containing the token
 * @return Newly allocated string with the token (CALLER MUST FREE), or NULL
 *         if no token is available
 */
char* copy_bearer_token(TOKEN_DATA* token_data) {
    char* copy = NULL;

    pthread_mutex_lock(&token_data->lock);
    if (token_data->token) copy = strdup(token_data->token);
    else fprintf(stderr, "ERROR - The bearer token is not valid to copy in copy_bearer_token()!\n");
    pthread_mutex_unlock(&token_data->lock);

    return copy;
}

#include <pthread.h>

#ifndef _AUTH_H_
#define _AUTH_H_

typedef struct TokenData {
    const char* api_key;
    char* token;

    pthread_cond_t received;
    pthread_cond_t completed;

    pthread_mutex_t lock;
} TOKEN_DATA;

#endif

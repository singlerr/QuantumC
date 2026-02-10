#ifndef _AUTH_H_
#define _AUTH_H_

#include <pthread.h>

typedef struct TokenContext {
    char* token;
    pthread_con_t received;
    pthread_mutex_t lock;
} TOKEN_CONTEXT;

#endif

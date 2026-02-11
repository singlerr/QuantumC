#ifndef _AUTH_H_
#define _AUTH_H_

#define MIN_TIME 300

typedef struct TokenData {
    const char* api_key;
    char* token;

    pthread_cond_t received;
    pthread_cond_t completed;

    pthread_mutex_t lock;
} TOKEN_DATA;

void* start_authenticator(void* arg);

#endif

#ifndef _AUTH_H_
#define _AUTH_H_

#define TTIME_OFFSET 300

typedef struct TokenData {
    const char* api_key;
    char* token;

    pthread_cond_t received;
    pthread_cond_t completed;

    pthread_mutex_t lock;
} TOKEN_DATA;

void update_bearer_token(TOKEN_DATA* token_data, char* token);
int get_bearer_token(TOKEN_DATA* token_data);

void* authenticator(void* arg);

#endif

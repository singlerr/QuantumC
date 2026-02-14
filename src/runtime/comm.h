#ifndef _COMM_H_
#define _COMM_H_

#define BUFFER_NMEMB 4096
#define USER_AGENT_NAME "QuantumC/dev"

typedef struct ResponseBuffer {
    char* data;
    size_t size;
} RESPONSE_BUFFER;

typedef struct TokenData {
    char* api_key;
    char* token;

    pthread_cond_t token_received;
    pthread_cond_t job_terminated;

    pthread_mutex_t lock;
} TOKEN_DATA;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

char* copy_bearer_token(TOKEN_DATA* token_data);

#endif

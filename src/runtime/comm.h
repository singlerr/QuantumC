#ifndef _COMM_H_
#define _COMM_H_

#define BUFFER_NMEMB 2048
#define USER_AGENT_NAME "QuantumC/dev"

typedef struct ResponseBuffer {
    char* data;
    size_t size;
} RESPONSE_BUFFER;

typedef struct TokenData {
    char* key;
    char* token;

    bool token_received_bool;
    bool job_terminated_bool;

    pthread_cond_t token_received_cond;
    pthread_cond_t job_terminated_cond;

    pthread_mutex_t lock;
} TOKEN_DATA;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

void initialize_token_data(TOKEN_DATA* token_data, char* key);
void destroy_token_data(TOKEN_DATA* token_data);

void signal_token_received(TOKEN_DATA* token_data);
void signal_job_terminated(TOKEN_DATA* token_data);

char* copy_bearer_token(TOKEN_DATA* token_data);

#endif

#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

typedef struct ResponseBuffer {
    char* data;
    size_t size;
} RESPONSEBUFFER;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

char* ibm_authentication(const char* api_key);

#endif

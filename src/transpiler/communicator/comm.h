#ifndef _COMM_H_
#define _COMM_H_

#define BUFFER_NMEMB 4096

typedef struct ResponseBuffer {
    char* data;
    size_t size;
} RESPONSE_BUFFER;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

#endif

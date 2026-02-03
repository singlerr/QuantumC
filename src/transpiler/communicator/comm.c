#include <stdlib.h>
#include <string.h>

#include "comm.h"


size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    RESPONSE_BUFFER *rb = userp;

    char *temp = realloc(rb->data, rb->size + real_size + 1);
    if (!temp) return 0;

    rb->data = temp;
    memcpy(rb->data+rb->size, contents, real_size);
    rb->size += real_size;
    rb->data[rb->size] = '\0';

    return real_size;
}

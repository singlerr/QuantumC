#include "stringlib.h"
#include <stdlib.h>
#include <string.h>

#if !defined(_SVID_SOURCE) || _POSIX_C_SOURCE == 200809L
char *
strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    void *new = malloc(len);
    if (new == NULL)
        return NULL;
    return (char *)memcpy(new, s, len);
}
#endif

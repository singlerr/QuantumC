#include "filebuf.h"
#include <stdlib.h>
#include <string.h>

FILEBUF *curbuf = NULL;

#define ALLOCBUF(size) ((char *)malloc(size))
#define AVAILABLE(fbuf) (fbuf && fbuf->buf)
#define SAFE_FREE(ptr) \
    do                 \
    {                  \
        if (ptr)       \
            free(ptr); \
    } while (0)

void init_buf()
{
    if (curbuf)
    {
        SAFE_FREE(curbuf->buf);
        free(curbuf);
    }

    curbuf = (FILEBUF *)malloc(sizeof(FILEBUF));
    curbuf->buf = ALLOCBUF(BUFCHNK);
    curbuf->buf[0] = '\0';
    curbuf->bufsize = BUFCHNK;
    curbuf->pos = 0;
}

int readchar(FILE *fin)
{
    if (!AVAILABLE(curbuf))
    {
        perror("readchar() called even if current buffer is not allocated");
    }

    if (curbuf->pos + 1 >= curbuf->bufsize)
    {
        curbuf->buf = (char *)realloc(curbuf->buf, curbuf->bufsize + BUFCHNK);
        curbuf->bufsize += BUFCHNK;
    }

    int c = fgetc(fin);
    curbuf->buf[curbuf->pos++] = c;
    curbuf->buf[curbuf->pos] = '\0';

    return c;
}

int getbuf(char **buf)
{
    if (!AVAILABLE(curbuf))
    {
        perror("getbuf() called even if current buffer is not allocated");
    }
    int len = curbuf->pos;

    char *b = (char *)malloc(len + 1);
    strncpy(b, curbuf->buf, len);
    b[len] = '\0';

    *buf = b;

    if (curbuf->bufsize == BUFCHNK)
    {
        curbuf->pos = 0;
    }
    else
    {
        free(curbuf->buf);
        curbuf->pos = 0;
        curbuf->bufsize = BUFCHNK;
        curbuf->buf = ALLOCBUF(BUFCHNK);
    }

    return len;
}

int peekbuf(char **buf)
{
    if (!AVAILABLE(curbuf))
    {
        perror("peekbuf() called even if current buffer is not allocated");
    }

    *buf = curbuf->buf;
    return curbuf->pos;
}
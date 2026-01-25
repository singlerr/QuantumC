#ifndef _COMMON_H_
#define _COMMON_H_

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#define VAL_OK 0
#define VAL_CONTINUE -2
#define VAL_FAILED -1

#define STG_EXTERN (1)
#define STG_STATIC (1 << 1)
#define STG_AUTO (1 << 2)
#define STG_REGISTER (1 << 3)
#define STG_TYPEDEF (1 << 4)

#define QAL_CONST (1)
#define QAL_RESTRICT (1 << 1)
#define QAL_VOLATILE (1 << 2)

#define OK(ret) (ret == VAL_OK)
#define FAILED(ret) (ret == VAL_FAILED)
#define CONTINUE(ret) (ret == VAL_CONTINUE)

#define ALLOC(type) ((type *)malloc(sizeof(type)))
#define IALLOC(type) ((type *)calloc(1, sizeof(type)))

#define SAFE_FREE(ptr) \
    do                 \
    {                  \
        if (ptr)       \
            free(ptr); \
    } while (0)

#endif
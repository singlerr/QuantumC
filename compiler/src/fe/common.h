#ifndef _COMMON_H_
#define _COMMON_H_

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#define VAL_OK 0
#define VAL_CONTINUE -2
#define VAL_FAILED -1

#define OK(ret) (ret == VAL_OK)
#define FAILED(ret) (ret == VAL_FAILED)
#define CONTINUE(ret) (ret == VAL_CONTINUE)

#define ALLOC(type) ((type *)malloc(sizeof(type)))

#endif
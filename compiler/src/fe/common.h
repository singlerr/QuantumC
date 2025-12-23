#ifndef _COMMON_H_
#define _COMMON_H_

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#define VAL_OK 0
#define VAL_FAILED -1
#define OK(ret) (ret == 0)
#define FAILED(ret) (ret < 0)

#define ALLOC(type) ((type)malloc(sizeof(type)))

#endif
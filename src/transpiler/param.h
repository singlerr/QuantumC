#ifndef _PARAM_H_
#define _PARAM_H_
#define SYM_MAXLEN 2048
#define CONSTR_STATIC (1)
#define CONSTR_CONST (1 << 1)
#define CONSTR_RESTRICT (1 << 2)
#define CONSTR_VOLATILE (1 << 3)
#define CONSTR_TYPEDEF (1 << 4)
#define CONSTR_EXTERN (1 << 5)
#define CONSTR_STATIC (1 << 6)
#define CONSTR_AUTO (1 << 7)
#define CONSTR_REGISTER (1 << 8)
#define CONSTR_EMPTY 0
#endif
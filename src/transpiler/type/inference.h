#ifndef _INFERENCE_H_
#define _INFERENCE_H_

typedef enum type_error
{
  OK,
  UNHANDLED_SYNTAX_NODE,
  UNDEFINED_SYMBOL,
  RECURSIVE_UNIFICATION,
} type_error;

#endif
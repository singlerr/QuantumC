#ifndef _AST_TYPING_H_
#define _AST_TYPING_H_

#include "type.h"
#include "common.h"

BOOL is_casting_compatible(const type_t *caster, const type_t *castee);
BOOL is_type_compatible(const type_t *left, const type_t *right);
BOOL is_param_compatible(const type_t *input, const type_t *param);
#endif
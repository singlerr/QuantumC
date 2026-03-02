#ifndef _TYTAB_H_
#define _TYTAB_H_

#include "base.h"

#define Type(TAG) new_simple_type (16, TAG)

const type_t *find_type_by_name (const char *name);
void push_type (const char *name, const type_t *origin);

#endif
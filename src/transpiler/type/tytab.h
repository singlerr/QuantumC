#ifndef _TYTAB_H_
#define _TYTAB_H_

#include "base.h"

#define Type(size, tag) new_simple_type (size, tag)

type_t *find_type_by_name (const char *name);
void push_type (const char *name, type_t *origin);
void init_type_table (void);

#endif
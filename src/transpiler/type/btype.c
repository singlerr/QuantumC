#include "base.h"

#define STRING_GEN(STRING) #STRING,
const char *ty_to_str[] = { FOREACH_TYPE (STRING_GEN) };
#undef STRING_GEN

const char *
type_to_str (type_tag_t tag)
{
  return ty_to_str[tag];
}
#include "stringbuilder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRSZ(str) (strlen (str) + 1)

void
init_str_builder (struct string_builder *builder)
{
  if (!builder)
    {
      return;
    }

  builder->buffer = NULL;
  builder->size = 0;
}
void
str_append (struct string_builder *builder, const char *str)
{
  if (!str)
    {
      return;
    }
  int len = strlen (str);
  if (!builder->buffer || builder->size == 0)
    {
      builder->buffer = (char *)malloc (len + 1);
      builder->size = len + 1;
    }
  else
    {
      int newsz = builder->size + len + 1;
      builder->buffer = (char *)realloc (builder->buffer, newsz);
      strncpy (builder->buffer + (builder->size - 1), str, len);
      builder->size = newsz;
    }
}

void
int_append (struct string_builder *builder, int i)
{
  char buf[2048];
  snprintf (buf, sizeof (buf), "%d", i);

  str_append (builder, buf);
}

void
float_append (struct string_builder *builder, float f)
{
  char buf[2048];
  snprintf (buf, sizeof (buf), "%f", f);
  str_append (builder, buf);
}

char *
end_str_builder (struct string_builder *builder)
{
  return builder->buffer;
}

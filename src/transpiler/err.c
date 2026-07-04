#include "err.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
void
error (char *str, ...)
{
  va_list args;
  va_start (args, str);
  vprintf (stderr, str, args);
  va_end (args);
  exit (1);
}

void
warn (const char *str, ...)
{
  va_list args;
  va_start (args, str);
  vprintf (stdout, str, args);
  va_end (args);
}
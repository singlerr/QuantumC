#include "err.h"
#include <stdio.h>
#include <stdlib.h>
void error(char *str)
{
    fprintf(stderr, str);
    exit(1);
}
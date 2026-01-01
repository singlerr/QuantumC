#include "diagnostics.h"
#include <stdio.h>
#include <stdarg.h>

extern char *yyfilename;
extern int line_number;
extern int column;

const char *log_names[] = {
    [INFO] = "info",
    [WARN] = "warn",
    [ERROR] = "error"};

void _log(log_level level, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    printf("[%s] %s:%d : ", log_names[level], yyfilename, line_number);
    printf(msg, args);
    printf("\n");
    va_end(args);
}
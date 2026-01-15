#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern char *yyfilename;
extern int line_number;
extern int column;

const char *log_names[] = {
    [INFO] = "info",
    [WARN] = "warn",
    [ERROR] = "error"};

static void __log(log_level level, const char *msg, va_list args)
{
    printf("[%s] %s:%d : ", log_names[level], yyfilename, line_number);
    vprintf(msg, args);
    printf("\n");
}

void _log(log_level level, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    __log(level, msg, args);
    va_end(args);
}

void _log_and_kill(log_level level, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    __log(level, msg, args);
    va_end(args);

    exit(1);
}
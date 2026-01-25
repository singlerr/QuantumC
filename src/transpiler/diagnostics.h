#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

typedef enum _log_level
{
    WARN = 0,
    ERROR,
    INFO
} log_level;

void _log(log_level level, const char *msg, ...);
void _log_and_kill(log_level level, const char *msg, ...);

#define LOG_ERROR(msg, ...) _log(ERROR, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) _log(WARN, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) _log(INFO, msg, ##__VA_ARGS__)

#define P_ERROR(msg, ...) _log_and_kill(ERROR, msg, ##__VA_ARGS__)

#endif
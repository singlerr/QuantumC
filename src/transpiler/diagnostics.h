#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

typedef enum _log_level
{
  WARN = 0,
  ERROR,
  INFO
} log_level;

void _log (log_level level, int lineno, const char *msg, ...);
void _log_and_kill (log_level level, int lineno, const char *msg, ...);

#define LOG_ERROR(msg, ...) _log (ERROR, 0, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) _log (WARN, 0, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) _log (INFO, 0, msg, ##__VA_ARGS__)

#define P_ERROR(msg, ...) _log_and_kill (ERROR, 0, msg, ##__VA_ARGS__)

#endif
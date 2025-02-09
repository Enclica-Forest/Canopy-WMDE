#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// Simple logging function that writes to stdout.
static inline void log_message(const char *level, const char *format, ...) {
    time_t now = time(NULL);
    char time_buf[20];
    struct tm *tm_info = localtime(&now);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(stdout, "[%s] [%s] ", time_buf, level);

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    fprintf(stdout, "\n");
}

#define LOG_INFO(...)  log_message("INFO", __VA_ARGS__)
#define LOG_WARN(...)  log_message("WARN", __VA_ARGS__)
#define LOG_ERROR(...) log_message("ERROR", __VA_ARGS__)

#endif
